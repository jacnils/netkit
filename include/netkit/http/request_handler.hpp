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

#ifdef NETKIT_HTTP

#include <algorithm>
#include <fstream>
#include <ranges>

#include <netkit/tcp/tcp_stream.hpp>
#include <netkit/body/buffer_body.hpp>
#include <netkit/http/basic_request_handler.hpp>
#include <netkit/http/predefined.hpp>
#include <netkit/network/utility.hpp>
#include <netkit/utility.hpp>
#include <netkit/except.hpp>
#include <netkit/body/async_stream_body.hpp>
#include <netkit/body/async_chunked_body.hpp>
#include <netkit/body/chunked_body.hpp>
#include <netkit/stream/utility.hpp>

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

        [[nodiscard]] static std::unordered_map<std::string, std::string>
        get_headers(const std::string& header_part) {
	        std::unordered_map<std::string, std::string> headers_map;

	        auto trim = [](std::string& s) {
	            auto start = s.find_first_not_of(" \t");

	            if (start == std::string::npos) {
	                s.clear();
	                return;
	            }

	            auto end = s.find_last_not_of(" \t");

	            s = s.substr(start, end - start + 1);
	        };

	        auto lowercase = [](std::string& s) {
	            std::ranges::transform(
                    s,
                    s.begin(),
                    [](unsigned char c) {
                        return static_cast<char>(std::tolower(c));
                    }
                );
	        };

	        std::istringstream hs(header_part);
	        std::string line;

	        while (std::getline(hs, line)) {
	            if (!line.empty() && line.back() == '\r')
	                line.pop_back();

	            if (line.empty())
	                break;

	            auto colon = line.find(':');

	            if (colon == std::string::npos)
	                continue;

	            auto key = line.substr(0, colon);
	            auto value = line.substr(colon + 1);

	            trim(key);
	            trim(value);

	            lowercase(key);

	            headers_map[key] = value;
	        }

	        return headers_map;
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
        netkit::io::task<void>
        handle(std::unique_ptr<tcp::async_tcp_stream> client_sock, server_settings& settings, const async_request_callback& callback) const override {
            if (!client_sock) {
                co_return;
            }

            bool close = false;

            while (!close) {
                if (settings.read_from_session_file == nullptr) {
                    settings.read_from_session_file = default_read_from_session_file;
                }
                if (settings.write_to_session_file == nullptr) {
                    settings.write_to_session_file = default_write_to_session_file;
                }

                async_request req{};
                auto [headers, overflow] = co_await stream::read_until(*client_sock, "\r\n\r\n");
                if (headers.empty()) {
                    co_return;
                }

                const auto header_map = get_headers(headers);

                bool is_chunked = false;
                std::size_t content_length = 0;

                auto status_line = get_status_line(headers);
                req.method = status_line.method;

                if (header_map.contains("transfer-encoding")) {
                    auto encoding = header_map.at("transfer-encoding");

                    if (encoding.find("chunked") != std::string::npos)
                        is_chunked = true;
                }

                if (header_map.contains("content-length")) {
                    try {
                        content_length = std::stoull(header_map.at("content-length"));
                    }
                    catch (...) {
                        throw parsing_error("invalid Content-Length");
                    }
                }

                if (header_map.contains("connection")) {
                    if (header_map.at("connection").find("close") != std::string::npos)
                        close = true;
                }

                if (header_map.contains("expect")) {
                    if (header_map.at("expect").find("100-continue") != std::string::npos)
                        co_await client_sock->write_all("HTTP/1.1 100 Continue\r\n\r\n");
                    else {
                        std::string response = "HTTP/1.1 417 Expectation Failed\r\n"
                            "Content-Length: 0\r\n"
                            "Connection: close\r\n"
                            "\r\n";

                        co_await client_sock->write_all(response);
                        co_return;
                    }
                }

                if (is_chunked && (req.method == "POST" || req.method == "PUT" || req.method == "PATCH" || req.method == "DELETE")) {
                    req.headers = header_map;
                    req.body = std::make_unique<netkit::body::async_chunked_body>(*client_sock, std::move(overflow));
                } else if (req.method == "POST" || req.method == "PUT" || req.method == "PATCH" || req.method == "DELETE") {
                	req.headers = header_map;
                	req.body = std::make_unique<netkit::body::async_stream_body>(*client_sock, content_length, std::move(overflow));
                } else {
                    req.headers = header_map;
                }

                req.ip_address = [&]() -> std::string {
                    if (settings.trust_x_forwarded_for) {
                        for (const auto& it : header_map) {
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
                    req.ip_address = client_sock->peer().get_ip();
                }

                if (!netkit::network::is_ipv4(req.ip_address) && !netkit::network::is_ipv6(req.ip_address)) {
                    throw parsing_error("invalid IP address: " + req.ip_address);
                }

                if (std::ranges::find(settings.blacklisted_ips, req.ip_address) != settings.blacklisted_ips.end()) {
                    co_return;
                }

                req.version = status_line.http_version == "HTTP/1.0" ? 10 : 11;

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

            	  for (const auto& it : header_map) {
                    if (it.first == "content-type") {
                        req.content_type = it.second;
                    } else if (it.first == "user-agent") {
                        req.user_agent = it.second;
                    } else if (it.first == "cookie") {
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

                auto response = co_await callback(req);

                std::stringstream header_section;

                header_section << "HTTP/1.1 " << response.http_status << " " << netkit::http::get_message(response.http_status).value_or("Unknown") << "\r\n";

                if (!response.content_type.empty()) header_section << "Content-Type: " << response.content_type << "\r\n";
                if (!response.allow_origin.empty()) header_section << "Access-Control-Allow-Origin: " << response.allow_origin << "\r\n";
                if (!response.location.empty()) {
                    header_section << "Location: " << response.location << "\r\n";
                }

                if (!response.headers.empty()) {
                    for (const auto& it : response.headers) {
                        header_section << it.name << ": " << it.data << "\r\n";
                    }
                }

                if (response.redirection == redirect_type::temporary) {
                    header_section << "Cache-Control: no-cache\r\n";
                } else if (response.redirection == redirect_type::permanent) {
                    header_section << "Cache-Control: no-store\r\n";
                }

                if (!session_id_found && settings.enable_session) {
                    session_id = netkit::utility::generate_random_string();
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
                        cookie_str += "Expires=" + netkit::utility::convert_unix_millis_to_gmt(it.expires) + "; ";
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

                    header_section << "Set-Cookie: " << cookie_str << "\r\n";
                }

                if (erase_associated) {
                    for (const auto& it : settings.associated_session_cookies) {
                        response.delete_cookies.push_back(it);
                    }
                }

                for (const auto& it : response.delete_cookies) {
                    std::string cookie_str = it + "=; Expires=Thu, 01 Jan 1970 00:00:00 GMT; ";
                    header_section << "Set-Cookie: " << cookie_str << "\r\n";
                }

                if (response.stop) {
                    co_return;
                }

                for (const auto& it : response.headers) {
                    if (it.name == "Content-Length") {
                        continue;
                    }
                    header_section << it.name << ": " << it.data << "\r\n";
                }

                if (close) {
                    header_section << "Connection: close\r\n";
                }

            	header_section << "Content-Length: " << response.body->size().value_or(0) << "\r\n";
                header_section << "\r\n";

            	co_await client_sock->write_all(header_section.str());

            	char buf[4096];

            	while (true) {
            		auto result = co_await response.body->read(buf, sizeof(buf));

            		using status_t = netkit::body::read_status;

            		switch (result.get_status()) {
            		case status_t::error:
            			throw std::runtime_error("Body read error");

            		case status_t::timeout:
            			continue;

            		case status_t::ok:
            		case status_t::eof: {
            			auto bytes = result.get_bytes_read();

            			if (bytes > 0) {
            				std::size_t total_sent = 0;

            				while (total_sent < bytes) {
            					auto [sent, write_status] =
            					    co_await client_sock->write_all(std::span<const std::byte>(
                                        reinterpret_cast<const std::byte*>(buf + total_sent),
                                    bytes - total_sent
                                    ));

            					if (write_status == netkit::stream::stream_status::error)
            						throw std::runtime_error("Socket write error");

            					total_sent += sent;
            				}
            			}

            			if (result.get_status() == status_t::eof)
            				co_return; // done

            			break;
            		}
                    default: break;
                    }
            	}
            }

            client_sock->close();
        }

        void handle(std::unique_ptr<tcp::tcp_stream> client_sock, server_settings& settings, const request_callback& callback) const override {
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
                auto [headers, overflow] = netkit::stream::read_until(*client_sock, "\r\n\r\n");
                if (headers.empty()) {
                    return;
                }

                const auto header_map = get_headers(headers);

                bool is_chunked = false;
                std::size_t content_length = 0;

                auto status_line = get_status_line(headers);
                req.method = status_line.method;

                if (header_map.contains("transfer-encoding")) {
                    auto encoding = header_map.at("transfer-encoding");

                    if (encoding.find("chunked") != std::string::npos)
                        is_chunked = true;
                }

                if (header_map.contains("content-length")) {
                    try {
                        content_length = std::stoull(header_map.at("content-length"));
                    }
                    catch (...) {
                        throw parsing_error("invalid Content-Length");
                    }
                }

                if (header_map.contains("connection")) {
                    if (header_map.at("connection").find("close") != std::string::npos)
                        close = true;
                }

                if (header_map.contains("expect")) {
                    if (header_map.at("expect").find("100-continue") != std::string::npos)
                        client_sock->write_all("HTTP/1.1 100 Continue\r\n\r\n");
                    else {
                        std::string response = "HTTP/1.1 417 Expectation Failed\r\n"
                            "Content-Length: 0\r\n"
                            "Connection: close\r\n"
                            "\r\n";

                        client_sock->write_all(response);
                        close = true;
                        break;
                    }
                }

                if (is_chunked && (req.method == "POST" || req.method == "PUT" || req.method == "PATCH" || req.method == "DELETE")) {
                    req.headers = header_map;
                    req.body = std::make_unique<netkit::body::chunked_body>(*client_sock, std::move(overflow));
                } else if (req.method == "POST" || req.method == "PUT" || req.method == "PATCH" || req.method == "DELETE") {
                	req.headers = header_map;
                	req.body = std::make_unique<netkit::body::stream_body>(*client_sock, content_length, std::move(overflow));
                } else {
                    req.headers = header_map;
                }

                req.ip_address = [&]() -> std::string {
                    if (settings.trust_x_forwarded_for) {
                        for (const auto& it : header_map) {
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
                    req.ip_address = client_sock->peer().get_ip();
                }

                if (!netkit::network::is_ipv4(req.ip_address) && !netkit::network::is_ipv6(req.ip_address)) {
                    throw parsing_error("invalid IP address: " + req.ip_address);
                }

                if (std::ranges::find(settings.blacklisted_ips, req.ip_address) != settings.blacklisted_ips.end()) {
                    return;
                }

                req.version = status_line.http_version == "HTTP/1.0" ? 10 : 11;

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

            	  for (const auto& it : header_map) {
                    if (it.first == "content-type") {
                        req.content_type = it.second;
                    } else if (it.first == "user-agent") {
                        req.user_agent = it.second;
                    } else if (it.first == "cookie") {
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

                std::stringstream header_section;

                header_section << "HTTP/1.1 " << response.http_status << " " << netkit::http::get_message(response.http_status).value_or("Unknown") << "\r\n";

                if (!response.content_type.empty()) header_section << "Content-Type: " << response.content_type << "\r\n";
                if (!response.allow_origin.empty()) header_section << "Access-Control-Allow-Origin: " << response.allow_origin << "\r\n";
                if (!response.location.empty()) {
                    header_section << "Location: " << response.location << "\r\n";
                }

                if (!response.headers.empty()) {
                    for (const auto& it : response.headers) {
                        header_section << it.name << ": " << it.data << "\r\n";
                    }
                }

                if (response.redirection == redirect_type::temporary) {
                    header_section << "Cache-Control: no-cache\r\n";
                } else if (response.redirection == redirect_type::permanent) {
                    header_section << "Cache-Control: no-store\r\n";
                }

                if (!session_id_found && settings.enable_session) {
                    session_id = netkit::utility::generate_random_string();
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
                        cookie_str += "Expires=" + netkit::utility::convert_unix_millis_to_gmt(it.expires) + "; ";
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

                    header_section << "Set-Cookie: " << cookie_str << "\r\n";
                }

                if (erase_associated) {
                    for (const auto& it : settings.associated_session_cookies) {
                        response.delete_cookies.push_back(it);
                    }
                }

                for (const auto& it : response.delete_cookies) {
                    std::string cookie_str = it + "=; Expires=Thu, 01 Jan 1970 00:00:00 GMT; ";
                    header_section << "Set-Cookie: " << cookie_str << "\r\n";
                }

                if (response.stop) {
                    return;
                }

                for (const auto& it : response.headers) {
                    if (it.name == "Content-Length") {
                        continue;
                    }
                    header_section << it.name << ": " << it.data << "\r\n";
                }

                if (close) {
                    header_section << "Connection: close\r\n";
                }

            	header_section << "Content-Length: " << response.body->size().value_or(0) << "\r\n";
                header_section << "\r\n";

                client_sock->write_all(header_section.str());

            	char buf[4096];

            	while (true) {
            		auto result = response.body->read(buf, sizeof(buf));

            		using status_t = netkit::body::read_status;

            		switch (result.get_status()) {
            		case status_t::error:
            			throw std::runtime_error("Body read error");

            		case status_t::timeout:
            			continue;

            		case status_t::ok:
            		case status_t::eof: {
            			auto bytes = result.get_bytes_read();

            			if (bytes > 0) {
            				std::size_t total_sent = 0;

            				while (total_sent < bytes) {
            					auto [sent, write_status] =
            					    client_sock->write_all(std::span<const std::byte>(
                                        reinterpret_cast<const std::byte*>(buf + total_sent),
                                    bytes - total_sent
                                    ));

            					if (write_status == netkit::stream::stream_status::error)
            						throw std::runtime_error("Socket write error");

            					total_sent += sent;
            				}
            			}

            			if (result.get_status() == status_t::eof)
            				return; // done

            			break;
            		}
                    default: break;
                    }
            	}
            }

            client_sock->close();
        }
    };
}

#endif