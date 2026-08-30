#pragma once

#include <algorithm>
#include <string>

#include <netkit/http/predefined.hpp>
#include <netkit/socket/addr.hpp>

#include <netkit/stream/wolfssl/async_tls_stream.hpp>
#include <netkit/http/header.hpp>
#include <netkit/stream/utility.hpp>
#include <netkit/body/async_chunked_body.hpp>
#include <netkit/body/async_stream_body.hpp>

#ifdef NETKIT_SSL
#include <netkit/tcp/async_tcp_stream.hpp>
#endif

namespace netkit::http {
    class async_client {
        socket::addr addr;
        netkit::io::io_context& ctx;
        scheme scheme_;
        std::unique_ptr<stream::basic_async_stream> stream;

        struct settings {
            std::string user_agent = "netkit/0.1";
            std::string accept = "*/*";
            std::string content_type = "application/octet-stream";
            bool close = false;
        } settings;

        netkit::io::task<void> connect() {
            if (stream && stream->is_open())
                co_return;

            auto sockstream =
                std::make_unique<tcp::async_tcp_stream>(ctx, addr);

            co_await sockstream->connect();

#ifdef NETKIT_SSL
            if (scheme_ == scheme::https) {
                auto tls =
                    std::make_unique<stream::async_tls_stream>(
                        std::move(sockstream),
                        stream::version::TLS_1_3,
                        stream::verification::none
                    );

                co_await tls->perform_handshake();

                stream = std::move(tls);
                co_return;
            }
#endif

            stream = std::move(sockstream);
        }

        void set_headers(netkit::http::headers& h, std::optional<size_t> size = std::nullopt) const {
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

            if (!settings.user_agent.empty())
                h.add("User-Agent", settings.user_agent);

            if (!settings.accept.empty())
                h.add("Accept", settings.accept);

            if (settings.close)
                h.add("Connection", "close");

            if (size.has_value()) {
                h.add("Content-Length", std::to_string(*size));

                if (!settings.content_type.empty())
                    h.add("Content-Type", settings.content_type);
            }
        }
    public:
        explicit async_client(netkit::io::io_context& ctx, const socket::addr& addr, const scheme scheme) : addr(addr), ctx(ctx), scheme_(scheme) {}

        void set_user_agent(const std::string& user_agent) {
            settings.user_agent = user_agent;
        }

        void set_accept(const std::string& accept) {
            settings.accept = accept;
        }

        void set_content_type(const std::string& content_type) {
            settings.content_type = content_type;
        }

        void set_close(bool close) {
            settings.close = close;
        }

        netkit::io::task<async_response> request(const std::string& method, const std::string& path, const std::unique_ptr<body::basic_async_body>& body, const headers& headers = {}) {
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

            set_headers(h, body ? body->size() : std::nullopt);

            for (auto& it : h) {
                ss << it.name.value() << ": " << it.value << "\r\n";
            }

            ss << "\r\n";

            co_await this->connect();

            co_await stream->write_all(ss.str());

            if (body && !body->empty()) {
                co_await stream->write_all(*body);
            }

            auto [header_data, overflow] = co_await netkit::stream::read_until(*stream, "\r\n\r\n");

            const auto line_end = header_data.find_first_of("\r\n");

            if (line_end == std::string::npos)
                throw std::logic_error{"invalid HTTP async_response"};

            const auto status_line = std::string_view{
                header_data.data(),
                line_end
            };

            const auto headers_data = std::string_view{
                header_data.data() + line_end + 2,
                header_data.size() - line_end - 2
            };


            async_response resp;

            resp.headers = parse_headers(headers_data.data());
            resp.status_code = parse_status_code(status_line);

            // TODO: parse transfer-encoding properly
            // TODO 2: gzip_body
            if (resp.headers.contains("transfer-encoding") && resp.headers.value("transfer-encoding") == "chunked") {
                resp.body = std::make_unique<netkit::body::async_chunked_body>(*stream, std::move(overflow));
            } else if (resp.headers.contains("content-length")) {
                const auto content_length = std::stoull(resp.headers.value("content-length"));

                if (content_length > 0) {
                    resp.body = std::make_unique<netkit::body::async_stream_body>(*stream, content_length, std::move(overflow));
                }
            } else {
                resp.body = std::make_unique<netkit::body::async_stream_body>(*stream, std::nullopt, std::move(overflow));
            }

            co_return resp;
        }

        io::task<async_response> request(method method, const std::string& path, const std::unique_ptr<body::basic_async_body>& body, const headers& headers = {}) {
            co_return co_await request(get_method_string(method), path, body, headers);
        }

        io::task<async_response> get(const std::string& path, const headers& headers = {}) {
            co_return co_await request(method::GET, path, nullptr, headers);
        }

        io::task<async_response> post(const std::string& path, const std::unique_ptr<body::basic_async_body>& body, const headers& headers = {}) {
            co_return co_await request(method::POST, path, body, headers);
        }

        io::task<async_response> put(const std::string& path, const std::unique_ptr<body::basic_async_body>& body, const headers& headers = {}) {
            co_return co_await request(method::PUT, path, body, headers);
        }

        io::task<async_response> patch(const std::string& path, const std::unique_ptr<body::basic_async_body>& body, const headers& headers = {}) {
            co_return co_await request(method::PATCH, path, body, headers);
        }
    };
}
