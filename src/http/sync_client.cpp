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
#ifdef NETKIT_HTTP

#include <netkit/except.hpp>
#include <netkit/http/sync_client.hpp>
#include <netkit/network/utility.hpp>
#include <netkit/tcp/tcp_stream.hpp>
#include <netkit/stream/wolfssl/tls_stream.hpp>
#include <netkit/utility.hpp>
#include <variant>

std::string netkit::http::client::sync_client::make_request(const std::string& request) const {
    sock::addr addr(hostname, port, sock::addr_type::hostname);

#if defined(NETKIT_SSL)
	using variant_sock = std::variant<netkit::tcp::tcp_stream, netkit::stream::tls_stream>;
#else
	using variant_sock = std::variant<netkit::tcp::tcp_stream>;
#endif

    std::optional<variant_sock> sock{std::nullopt};
#if defined(NETKIT_SSL)
    if (port == 443) {
        auto tcp_sock = std::make_unique<netkit::tcp::tcp_stream>(addr);
    	tcp_sock->connect();
        sock.emplace(std::in_place_type<netkit::stream::tls_stream>,
                     std::move(tcp_sock));
    	std::get<netkit::stream::tls_stream>(*sock).perform_handshake();
    } else {
    	sock.emplace(std::in_place_type<netkit::tcp::tcp_stream>, addr);
    	std::get<netkit::tcp::tcp_stream>(*sock).connect();
    }
#else
	sock.emplace(std::in_place_type<netkit::tcp::tcp_stream>, addr);
	std::get<netkit::tcp::tcp_stream>(*sock).connect();
#endif

    auto& s = *sock;

	const auto write_data = [&](const std::string& data) {
		std::visit([&](auto& socket) {
			socket.write_all(data);
		}, s);
	};

	const auto recv_data = [&]() -> std::string {
		std::string ret;
		std::visit([&](auto& socket) {
			ret = socket.read_all_string();
		}, s);
		return ret;
	};

	write_data(request);

    std::string raw;
    std::string s_headers;

    while (true) {
		raw += recv_data();

        if (auto pos = raw.find_first_of("\r\n\r\n"); pos != std::string::npos) {
            s_headers = raw.substr(0, pos + 4);
            raw = raw.substr(pos + 4);
            break;
        }
    }

    bool is_chunked = false;

    std::istringstream header_stream(s_headers);
    std::string line;
    while (std::getline(header_stream, line) && line != "\r") {
        if (line.starts_with("Transfer-Encoding:") && line.find("chunked") != std::string::npos) {
            is_chunked = true;
        }
    }

    std::string s_body;

    if (is_chunked) {
        std::string chunked_data = std::move(raw);
        s_body = utility::decode_chunked(chunked_data);
    } else {
        s_body = std::move(raw);
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

#endif