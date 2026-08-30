#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include <netkit/http/predefined.hpp>
#include <netkit/body/basic_body.hpp>
#include <netkit/socket/addr.hpp>
#include <netkit/stream/basic_stream.hpp>

#include <netkit/stream/wolfssl/tls_stream.hpp>
#include <netkit/body/stream_body.hpp>
#include <netkit/body/chunked_body.hpp>
#include <netkit/except.hpp>
#include <netkit/http/header.hpp>
#include <netkit/stream/utility.hpp>

#ifdef NETKIT_SSL
#include <netkit/tcp/tcp_stream.hpp>
#endif

namespace netkit::http {
    struct response {
        int status_code{};
        netkit::http::headers headers;
        std::unique_ptr<body::basic_body> body;
    };

    enum class scheme {
#ifdef NETKIT_SSL
        https,
#endif
        http
    };

    class client {
        socket::addr addr;
        scheme scheme_;
        std::unique_ptr<stream::basic_stream> stream;

        struct settings {
            std::string user_agent = "netkit/0.1";
            std::string accept = "*/*";
            std::string content_type = "application/octet-stream";
            bool close = false;
        } settings;

        void connect() {
            if (!stream || !stream->is_open()) {
#ifdef NETKIT_SSL
                if (scheme_ == scheme::https) {
                    auto sockstream = std::make_unique<tcp::tcp_stream>(addr);
                    sockstream->connect();

                    auto tls_sockstream = std::make_unique<stream::tls_stream>(std::move(sockstream));
                    tls_sockstream->perform_handshake();

                    stream = std::move(tls_sockstream);

                    return;
                }
#endif
                auto sockstream = std::make_unique<tcp::tcp_stream>(addr);
                sockstream->connect();
                stream = std::move(sockstream);
            }
        }

        static std::string get_method_string(method method) {
            std::stringstream ss;
            switch (method) {
            case method::GET:
                ss << "GET"; break;
            case method::HEAD:
                ss << "HEAD"; break;
            case method::POST:
                ss << "POST"; break;
            case method::PUT:
                ss << "PUT"; break;
            case method::DELETE:
                ss << "DELETE"; break;
            case method::CONNECT:
                ss << "CONNECT"; break;
            case method::OPTIONS:
                ss << "OPTIONS"; break;
            case method::TRACE:
                ss << "TRACE"; break;
            case method::PATCH:
                ss << "PATCH"; break;
            case method::undefined:
                throw std::logic_error("undefined method");
            }
            return ss.str();
        }

        void set_headers(netkit::http::headers& h, const std::unique_ptr<body::basic_body>& body) const {
            try {
                if (addr.get_port() != 80 && addr.get_port() != 443)
                    h.add("Host", addr.get_hostname() + ":" + std::to_string(addr.get_port()));
                else
                    h.add("Host", addr.get_hostname());
            } catch (...) {
                if (addr.get_port() != 80 && addr.get_port() != 443)
                    h.add("Host", addr.get_ip() + ":" + std::to_string(addr.get_port()));
                else
                    h.add("Host", addr.get_ip());
            }

            if (!settings.user_agent.empty()) {
                h.add("User-Agent", settings.user_agent);
            }

            if (!settings.accept.empty()) {
                h.add("Accept", settings.accept);
            }

            if (settings.close) {
                h.add("Connection", "close");
            }

            if (body && body->size().has_value()) {
                h.add("Content-Length", std::to_string(body->size().value()));
            } else if (body) {
                throw std::logic_error("cannot use this body type as of now");
            }

            if (body && !body->empty()) {
                h.add("Content-Type", settings.content_type);
            }
        }
    public:
        explicit client(const socket::addr& addr, const scheme scheme) : addr(addr), scheme_(scheme) {}

        response request(const std::string& method, const std::string& path, const std::unique_ptr<body::basic_body>& body, const headers& headers = {}) {
            std::stringstream ss;

            ss << method;

            if (path.empty()) {
                throw std::logic_error("path is empty");
            }

            if (path.at(0) != '/') {
                throw std::logic_error("path must start with /");
            }

            ss << " " << path << " HTTP/1.1" << "\r\n";

            netkit::http::headers h = headers;

            set_headers(h, body);

            for (auto& it : h) {
                ss << it.name.value() << ": " << it.value << "\r\n";
            }

            ss << "\r\n";

            this->connect();

            stream->write_all(ss.str());

            if (body && !body->empty()) {
                stream->write_all(*body);
            }

            auto [header_data, overflow] = netkit::stream::read_until(*stream, "\r\n\r\n");

            const auto line_end = header_data.find_first_of("\r\n");

            if (line_end == std::string::npos)
                throw std::logic_error{"invalid HTTP response"};

            const auto status_line = std::string_view{
                header_data.data(),
                line_end
            };

            const auto headers_data = std::string_view{
                header_data.data() + line_end + 2,
                header_data.size() - line_end - 2
            };

            response resp;

            resp.headers = parse_headers(headers_data.data());
            resp.status_code = parse_status_code(status_line);

            // TODO: parse transfer-encoding properly
            // TODO 2: gzip_body
            if (resp.headers.contains("transfer-encoding") && resp.headers.value("transfer-encoding") == "chunked") {
                resp.body = std::make_unique<netkit::body::chunked_body>(*stream, std::move(overflow));
            } else if (resp.headers.contains("content-length")) {
                const auto content_length = std::stoull(resp.headers.value("content-length"));

                if (content_length > 0) {
                    resp.body = std::make_unique<netkit::body::stream_body>(*stream, content_length, std::move(overflow));
                }
            } else {
                resp.body = std::make_unique<netkit::body::stream_body>(*stream, std::nullopt, std::move(overflow));
            }

            return resp;
        }

        response request(method method, const std::string& path, const std::unique_ptr<body::basic_body>& body, const headers& headers = {}) {
            return request(get_method_string(method), path, body, headers);
        }

        response get(const std::string& path, const headers& headers = {}) {
            return request(method::GET, path, nullptr, headers);
        }

        response post(const std::string& path, const std::unique_ptr<body::basic_body>& body, const headers& headers = {}) {
            return request(method::POST, path, body, headers);
        }

        response put(const std::string& path, const std::unique_ptr<body::basic_body>& body, const headers& headers = {}) {
            return request(method::PUT, path, body, headers);
        }

        response patch(const std::string& path, const std::unique_ptr<body::basic_body>& body, const headers& headers = {}) {
            return request(method::PATCH, path, body, headers);
        }
    };
}
