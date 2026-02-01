/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file sync_client.cpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Implementation of the synchronous HTTP client class.
 */
#include <netkit/http/sync_client.hpp>
#include <netkit/sock/openssl/ssl_sync_sock.hpp>
#include <netkit/utility.hpp>
#include <netkit/except.hpp>
#include <netkit/network/utility.hpp>

std::string netkit::http::client::sync_client::make_request(const std::string& request) const {
    sock::addr addr(hostname, port, sock::addr_type::hostname);

    std::optional<variant_sock> sock{std::nullopt};
#ifdef NETKIT_OPENSSL
    if (port == 443) {
        auto tcp_sock = std::make_unique<netkit::sock::sync_sock>(addr, netkit::sock::type::tcp);
        sock.emplace(std::in_place_type<netkit::sock::ssl_sync_sock>,
                     std::move(tcp_sock),
                     netkit::sock::ssl_sync_sock::mode::client);
    } else {
        sock.emplace(netkit::sock::sync_sock(addr, netkit::sock::type::tcp));
        std::get<netkit::sock::sync_sock>(*sock).connect();
    }
#else
    sock.emplace(sock::sync_sock(addr, sock::type::tcp));
    std::get<sock::sync_sock>(*sock).connect();
#endif

    auto& s = *sock;
    std::visit([](auto& sckt){ sckt.connect(); }, s);
    std::visit([&](auto& sckt) { sckt.send(request.data(), request.size()); }, s);

    std::string raw;
    std::string s_headers;
    while (true) {
        auto result = std::visit([&](auto& sckt) { return sckt.recv(timeout, "\r\n\r\n", 0); }, s);
        if (result.status == sock::recv_status::timeout) {
            throw std::runtime_error("timeout while reading headers");
        }
        if (result.status == sock::recv_status::closed) {
            throw std::runtime_error("connection closed during headers");
        }
        if (result.data.empty()) {
            throw std::runtime_error("empty recv data unexpectedly");
        }

        raw += result.data;
        if (auto pos = raw.find("\r\n\r\n"); pos != std::string::npos) {
            s_headers = raw.substr(0, pos + 4);
            raw = raw.substr(pos + 4);
            break;
        }
    }

    bool is_chunked = false;
    std::size_t content_length = 0;

    std::istringstream header_stream(s_headers);
    std::string line;
    while (std::getline(header_stream, line) && line != "\r") {
        if (line.starts_with("Transfer-Encoding:") && line.find("chunked") != std::string::npos) {
            is_chunked = true;
        } else if (line.starts_with("Content-Length:")) {
            content_length = std::stoul(line.substr(15));
        }
    }


    std::string s_body;

    if (is_chunked) {
        std::string chunked_data = std::move(raw);
        while (chunked_data.find("0\r\n\r\n") == std::string::npos) {
            std::string chunk = std::visit([&](auto& sckt) { return sckt.recv(timeout, 8192); }, s).data;
            if (chunk.empty()) throw std::runtime_error("connection closed during chunked body");
            chunked_data += chunk;
        }
        s_body = utility::decode_chunked(chunked_data);
    } else {
        s_body = std::move(raw);
        while (s_body.size() < content_length) {
            auto res = std::visit([&](auto& sckt) { return sckt.recv(30, "", 0); }, s);
            if (res.data.empty()) break;
            s_body += res.data;
        }
    }

    return s_headers + s_body;
}

netkit::http::client::sync_client::sync_client(const std::string& hostname, const std::string& path, int port, method m, version v, int timeout)
    : hostname(hostname), path(path), port(port), m(m), v(v), timeout(timeout) {
    if (!netkit::network::is_valid_port(port)) {
        throw parsing_error("invalid port");
    }
    if (hostname.empty()) {
        throw parsing_error("hostname is empty");
    }
    if (path.empty() || path[0] != '/') {
        throw parsing_error("path is empty");
    }

    this->method_str = (m == method::GET) ? "GET" : "POST";
    this->version_str = (v == version::HTTP_1_0) ? "HTTP/1.0" : "HTTP/1.1";
}

void netkit::http::client::sync_client::append_headers(const std::vector<std::pair<std::string, std::string>>& in_headers) {
    for (const auto& [key, value] : in_headers) {
        if (key == "Host" || key == "Content-Length") {
            throw parsing_error("illegal header: " + key);
        }
        this->headers.emplace_back(key, value);
    }
}

void netkit::http::client::sync_client::set_body(const std::string& in_body) {
    this->body = in_body;
}

void netkit::http::client::sync_client::set_header(const std::string& key, const std::string& value) {
    if (key == "Host" || key == "Content-Length") {
        throw netkit::parsing_error("illegal header: " + key);
    }
    this->headers.emplace_back(key, value);
}

void netkit::http::client::sync_client::set_user_agent(const std::string& user_agent) {
    this->set_header("User-Agent", user_agent);
}

void netkit::http::client::sync_client::set_content_type(const std::string& content_type) {
    this->set_header("Content-Type", content_type);
}

void netkit::http::client::sync_client::set_accept(const std::string& accept) {
    this->set_header("Accept", accept);
}

void netkit::http::client::sync_client::set_accept_encoding(const std::string& accept_encoding) {
    this->set_header("Accept-Encoding", accept_encoding);
}

void netkit::http::client::sync_client::set_accept_language(const std::string& accept_language) {
    this->set_header("Accept-Language", accept_language);
}

void netkit::http::client::sync_client::set_connection(const std::string& connection) {
    this->set_header("Connection", connection);
}

void netkit::http::client::sync_client::set_referer(const std::string& referer) {
    this->set_header("Referer", referer);
}

void netkit::http::client::sync_client::set_cache_control(const std::string& cache_control) {
    this->set_header("Cache-Control", cache_control);
}

void netkit::http::client::sync_client::set_cookie(const std::string& cookie) {
    this->set_header("Cookie", cookie);
}

void netkit::http::client::sync_client::set_connect_timeout(int in_timeout) {
    this->set_header("Connect-Timeout", std::to_string(in_timeout));
}

std::vector<std::pair<std::string, std::string>> netkit::http::client::sync_client::get_headers() const {
    return this->headers;
}

std::string netkit::http::client::sync_client::get_body() const {
    return this->body;
}

std::string netkit::http::client::sync_client::get_hostname() const {
    return this->hostname;
}

std::string netkit::http::client::sync_client::get_path() const {
    return this->path;
}

int netkit::http::client::sync_client::get_port() const {
    return this->port;
}

netkit::http::method netkit::http::client::sync_client::get_method() const {
    return this->m;
}

netkit::http::version netkit::http::client::sync_client::get_version() const {
    return this->v;
}