/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file request_handler.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides a handler for HTTP server requests.
 */
#pragma once

#include <ranges>
#include <algorithm>
#include <fstream>

#include <netkit/http/basic_request_handler.hpp>
#include <netkit/sock/sync_sock.hpp>
#include <netkit/network/utility.hpp>
#include <netkit/http/predefined.hpp>
#include <netkit/utility.hpp>

namespace netkit::http::server {
    template <typename S = server_settings>
    class request_handler : public basic_request_handler<> {
        static std::vector<cookie> get_cookies_from_request(const std::string& cookie_header) {
            std::vector<cookie> cookies;
            std::string cookie_str = cookie_header + ";";

            while (cookie_str.find(';') != std::string::npos) {
                std::string cookie = cookie_str.substr(0, cookie_str.find(';'));
                cookie_str = cookie_str.substr(cookie_str.find(';') + 1);

                std::string name = cookie.substr(0, cookie.find('='));
                std::string value = cookie.substr(cookie.find('=') + 1);

                if (!name.empty() && !value.empty()) {
                    if (name.front() == ' ') {
                        name = name.substr(1);
                    }
                    cookies.push_back({name, value});
                }
            }

            return cookies;
        }

        static std::unordered_map<std::string, std::string> default_read_from_session_file(const std::string& f) {
            std::unordered_map<std::string, std::string> session;

            std::ifstream file(f);

            if (!file.good()) {
                file.close();
                return {};
            }

            if (!file.is_open()) {
                throw std::runtime_error("failed to open session file (read_from_session_file()): " + f);
            }

            std::string line{};
            while (std::getline(file, line)) {
                if (line.find('=') != std::string::npos) {
                    std::string key = line.substr(0, line.find('='));
                    std::string value = line.substr(line.find('=') + 1);

                    session[key] = value;
                }
            }

            file.close();

            return session;
        }

        static void default_write_to_session_file(const std::string& f, const std::unordered_map<std::string, std::string>& session) {
            auto directory = std::filesystem::path(f).parent_path();
            if (!std::filesystem::exists(directory)) {
                std::filesystem::create_directories(directory);
            }
            std::ofstream file(f, std::ios::trunc);

            if (!file.is_open() || !file.good()) {
                throw std::runtime_error("failed to open session file (write_to_session_file()): " + f);
            }

            for (const auto& it : session) {
                file << it.first << "=" << it.second << "\n";
            }

            file.close();
        }

        [[nodiscard]] static std::vector<std::pair<std::string, std::string>> get_headers(const std::string& header_part) {
            std::vector<std::pair<std::string,std::string>> headers_vec;
            std::istringstream hs(header_part);
            std::string l{};
            while (std::getline(hs, l) && l != "\r") {
                if (l.back() == '\r') l.pop_back();
                auto cpos = l.find(':');
                if (cpos != std::string::npos) {
                    auto key = l.substr(0, cpos);
                    auto value = l.substr(cpos + 1);
                    auto trim = [](std::string& s) {
                        s.erase(0, s.find_first_not_of(" \t"));
                        s.erase(s.find_last_not_of(" \t") + 1);
                    };
                    trim(key);
                    trim(value);
                    headers_vec.emplace_back(key, value);
                }
            }

            return headers_vec;
        }

        struct status_line {
            std::string method{"GET"};
            std::string path{"/"};
            std::string http_version{"HTTP/1.1"};
        };

        status_line get_status_line(const std::string& header_part) const {
            status_line line{};
            std::istringstream hs(header_part);
            std::string first_line{};
            if (std::getline(hs, first_line)) {
                if (first_line.back() == '\r') first_line.pop_back();
                std::istringstream line_ss(first_line);
                line_ss >> line.method >> line.path >> line.http_version;
            }
            return line;
        }
    public:
        void handle(std::unique_ptr<sock::basic_sync_sock>& client_sock, server_settings& settings, const request_callback& callback) const override {
            if (!client_sock) {
                return;
            }

            bool close = false;

            while (!close) {
                if (settings.read_from_session_file == nullptr) {
                    settings.read_from_session_file = default_read_from_session_file;
                }
                if (settings.write_to_session_file == nullptr) {
                    settings.write_to_session_file = default_write_to_session_file;
                }

                request req{};
                std::string raw{};
                std::string headers = client_sock->recv(5, "\r\n\r\n").data;
                const auto headers_vec = get_headers(headers);
                if (headers.empty()) {
                    return;
                }
                raw += headers;

                bool is_chunked = false;
                std::size_t content_length = 0;

                auto status_line = get_status_line(headers);
                req.method = status_line.method;

                std::istringstream header_stream(headers);
                std::string line;
                while (std::getline(header_stream, line) && line != "\r") {
                    if (line.starts_with("Transfer-Encoding:") && line.find("chunked") != std::string::npos) {
                        is_chunked = true;
                    } else if (line.starts_with("Content-Length:")) {
                        try {
                            content_length = std::stoul(line.substr(15));
                        } catch (...) {
                            break;
                        }
                    } else if (line.starts_with("Expect:") && line.find("100-continue") != std::string::npos) {
                        client_sock->send("HTTP/1.1 100 Continue\r\n\r\n");
                    } else if (line.starts_with("Expect:") && line.find("100-continue") == std::string::npos) {
                        std::string response = "HTTP/1.1 417 Expectation Failed\r\n"
                            "Content-Length: 0\r\n"
                            "Connection: close\r\n"
                            "\r\n";

                        client_sock->send(response);
                        return;
                    } else if (line.starts_with("Upgrade:") && line.find("websocket") != std::string::npos) {
                        std::string response = "HTTP/1.1 426 Upgrade Required\r\n"
                            "Content-Length: 0\r\n"
                            "Connection: close\r\n"
                            "\r\n";

                        client_sock->send(response);
                        return;
                    } else if (line.starts_with("Connection:") && line.find("close") != std::string::npos) {
                        close = true;
                    }
                }

                if (is_chunked && (req.method == "POST" || req.method == "PUT" || req.method == "PATCH" || req.method == "DELETE")) {
                    std::string chunked = client_sock->overflow_bytes();
                    client_sock->clear_overflow_bytes();

                    while (chunked.find("0\r\n\r\n") == std::string::npos) {
                        auto res = client_sock->recv(5, "", 0); // no eof
                        if (res.status == sock::recv_status::closed) break;
                        if (res.status == sock::recv_status::timeout) close = true;
                        if (res.data.empty()) continue;
                        chunked += res.data;
                    }

                    std::string decoded = utility::decode_chunked(chunked);
                    raw = headers + decoded;
                    req.raw_body = raw;
                    req.body = decoded;
                } else if (req.method == "POST" || req.method == "PUT" || req.method == "PATCH" || req.method == "DELETE") {
                    std::string body = client_sock->overflow_bytes();
                    client_sock->clear_overflow_bytes();

                    while (body.size() < content_length) {
                        auto res = client_sock->recv(5, "", 0);
                        if (res.status == sock::recv_status::closed) break;
                        if (res.status == sock::recv_status::timeout) close = true;
                        if (res.data.empty()) continue;
                        body += res.data;
                    }

                    req.body = body;
                    req.raw_body = headers + body;
                } else {
                    req.raw_body = headers;
                }

                if (req.raw_body.empty() || req.raw_body.size() > settings.max_request_size) {
                    return;
                }

                req.ip_address = [&]() -> std::string {
                    if (settings.trust_x_forwarded_for) {
                        for (const auto& it : headers_vec) {
                            if (it.first == "X-Forwarded-For") {
                                auto ips = netkit::utility::split(it.second, ",");
                                for (const auto& ip : ips) {
                                    if (netkit::network::is_ipv4(ip) || netkit::network::is_ipv6(ip)) {
                                        return ip;
                                    }
                                }
                            }
                        }
                    }
                    return {};
                }();

                if (req.ip_address.empty()) {
                    req.ip_address = client_sock->get_peer().get_ip();
                }

                if (!netkit::network::is_ipv4(req.ip_address) && !netkit::network::is_ipv6(req.ip_address)) {
                    throw parsing_error("invalid IP address: " + req.ip_address);
                }

                if (std::ranges::find(settings.blacklisted_ips, req.ip_address) != settings.blacklisted_ips.end()) {
                    return;
                }

                req.version = [&]() {
                    if (status_line.http_version == "HTTP/1.0") {
                        return 10;
                    } else if (status_line.http_version == "HTTP/1.1") {
                        return 11;
                    } else {
                        throw parsing_error("unsupported HTTP version: " + status_line.http_version);
                    }
                }();
                auto full_path = status_line.path;
                if (full_path.empty() || full_path[0] != '/') {
                    throw parsing_error("invalid path: " + full_path);
                }
                auto query_pos = full_path.find('?');
                if (query_pos != std::string::npos) {
                    req.endpoint = full_path.substr(0, query_pos);
                    auto query_str = full_path.substr(query_pos + 1);
                    req.query = netkit::utility::parse_fields(query_str);
                } else {
                    req.endpoint = full_path;
                }

                req.fields = netkit::utility::parse_fields(req.body);
                for (const auto& it : headers_vec) {
                    if (it.first == "Content-Type") {
                        req.content_type = it.second;
                    } else if (it.first == "User-Agent") {
                        req.user_agent = it.second;
                    } else if (it.first == "Cookie") {
                        req.cookies = get_cookies_from_request(it.second);
                    }
                }

                std::string session_id{};
                bool session_id_found = false;
                for (const auto& it : req.cookies) {
                    if (it.name == settings.session_cookie_name && !it.value.empty() && settings.enable_session) {
                        session_id = it.value;
                        session_id_found = true;
                        break;
                    }
                }

                bool erase_associated = false;
                if (session_id_found) {
                    std::erase(session_id, '/');
                    std::filesystem::path session_file = settings.session_directory + "/session_" + session_id + ".txt";
                    req.session = settings.read_from_session_file(session_file.string());
                    req.session_id = session_id;

                    if (!std::filesystem::exists(session_file)) {
                        erase_associated = true;
                        // remove associated session cookies and session cookie from request
                        for (const auto& it : settings.associated_session_cookies) {
                            req.cookies.erase(
                                std::remove_if(req.cookies.begin(), req.cookies.end(),
                                               [&it](const cookie& cookie) {
                                                   return cookie.name == it;
                                               }),
                                req.cookies.end()
                            );
                        }
                        req.cookies.erase(
                            std::remove_if(req.cookies.begin(), req.cookies.end(),
                                           [this, &settings](const cookie& cookie) {
                                               return cookie.name == settings.session_cookie_name;
                                           }),
                            req.cookies.end()
                        );

                        req.session.clear();
                        req.session_id.clear();
                    } else {
                        req.session = settings.read_from_session_file(session_file.string());
                        req.session_id = session_id;
                    }
                }

                auto response = callback(req);
                std::stringstream net_response;
                net_response << "HTTP/1.1 " << response.http_status << " " << netkit::http::get_message(response.http_status).value_or("Unknown") << "\r\n";
                if (!response.content_type.empty()) net_response << "Content-Type: " << response.content_type << "\r\n";
                if (!response.allow_origin.empty()) net_response << "Access-Control-Allow-Origin: " << response.allow_origin << "\r\n";
                if (!response.location.empty()) {
                    net_response << "Location: " << response.location << "\r\n";
                }
                if (!response.headers.empty()) {
                    for (const auto& it : response.headers) {
                        net_response << it.name << ": " << it.data << "\r\n";
                    }
                }
                if (response.redirection == redirect_type::temporary) {
                    net_response << "Cache-Control: no-cache\r\n";
                } else if (response.redirection == redirect_type::permanent) {
                    net_response << "Cache-Control: no-store\r\n";
                }

                if (!session_id_found && settings.enable_session) {
                    session_id = utility::generate_random_string();
                    response.cookies.push_back({.name = settings.session_cookie_name, .value = session_id, .expires = 0, .path = "/", .same_site = "Strict", .http_only = true, .secure = settings.session_is_secure});
                } else if (settings.enable_session) {
                    std::string session_file = settings.session_directory + "/session_" + session_id + ".txt";
                    std::unordered_map<std::string, std::string> stored = settings.read_from_session_file(session_file);

                    for (const auto& it : response.session) {
                        stored[it.first] = it.second;
                    }

                    settings.write_to_session_file(session_file, stored);
                }

                for (const auto& it : response.cookies) {
                    std::string cookie_str = it.name + "=" + it.value + "; ";
                    if (it.expires != 0) {
                        cookie_str += "Expires=" + utility::convert_unix_millis_to_gmt(it.expires) + "; ";
                    } else {
                        cookie_str += "Expires=session; ";
                    }
                    if (it.http_only) {
                        cookie_str += "HttpOnly; ";
                    }
                    if (it.secure) {
                        cookie_str += "Secure; ";
                    }
                    if (!it.path.empty()) {
                        cookie_str += "Path=" + it.path + "; ";
                    }
                    if (!it.domain.empty()) {
                        cookie_str += "Domain=" + it.domain + "; ";
                    }
                    if (!it.same_site.empty() && (it.same_site == "Strict" || it.same_site == "Lax" || it.same_site == "None")) {
                        cookie_str += "SameSite=" + it.same_site + "; ";
                    }
                    for (const auto& attribute : it.attributes) {
                        cookie_str += attribute + "; ";
                    }
                    for (const auto& attribute : it.extra_attributes) {
                        cookie_str += attribute.first + "=" + attribute.second + "; ";
                    }

                    net_response << "Set-Cookie: " << cookie_str << "\r\n";
                }

                if (erase_associated) {
                    for (const auto& it : settings.associated_session_cookies) {
                        response.delete_cookies.push_back(it);
                    }
                }

                for (const auto& it : response.delete_cookies) {
                    std::string cookie_str = it + "=; Expires=Thu, 01 Jan 1970 00:00:00 GMT; ";
                    net_response << "Set-Cookie: " << cookie_str << "\r\n";
                }

                if (response.stop) {
                    return;
                }

                for (const auto& it : response.headers) {
                    if (it.name == "Content-Length") {
                        continue;
                    }
                    net_response << it.name << ": " << it.data << "\r\n";
                }

                if (close) {
                    net_response << "Connection: close\r\n";
                }

                if (!response.body.empty()) {
                    net_response << "Content-Length: " << response.body.size() << "\r\n";
                } else {
                    net_response << "Content-Length: 0\r\n";
                }

                net_response << "\r\n";
                net_response << response.body;

                client_sock->send(net_response.str());
            }

            client_sock->close();
        }
    };
}