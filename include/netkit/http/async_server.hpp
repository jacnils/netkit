/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file async_server.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides a synchronous HTTP server class template.
 *  @see netkit::http::server::basic_async_server
 */
#pragma once

#ifdef NETKIT_HTTP

#include <functional>
#include <netkit/except.hpp>
#include <netkit/http/basic_async_server.hpp>
#include <netkit/http/server_predefined.hpp>
#include <netkit/socket/addr.hpp>
#include <netkit/tcp/async_tcp_server.hpp>
#include <thread>

namespace netkit::http::server {
    /**
     * @brief  Class that represents a server.
     */
    template <typename T = request_handler<>>
    class async_server : public basic_async_server<> {
        bool running = true;
        server_settings settings;
        std::function<netkit::io::task<async_response>(const async_request&)> callback;
        std::unique_ptr<tcp::async_tcp_server> sock;
        io::io_context& ctx;
    public:
        /**
         * @brief  Constructor for the server class
         * @param  ctx      I/O context
         * @param  settings The settings for the server
         * @param  callback The function to call when a request is made
         */
        async_server(netkit::io::io_context& ctx, server_settings settings, const std::function<netkit::io::task<async_response>(const async_request&)>& callback)
            : settings(std::move(settings)), callback(callback), ctx(ctx)
        {
            if (!netkit::network::is_valid_port(settings.port)) {
                throw parsing_error("invalid port");
            }

            sock::addr addr = {"localhost", settings.port, netkit::sock::addr_type::hostname};
            this->sock = std::make_unique<tcp::async_tcp_server>(ctx, addr);

            try {
                sock->bind();
            } catch (const std::exception&) {
                throw socket_error("failed to bind socket, port might be in use");
            }
            sock->listen(settings.max_connections);
        };
        ~async_server() override {
            sock->close();
        }
        /**
         * @brief  Run the server
         */
        io::task<void> run() override {
            while (running) {
                auto client_sock = co_await sock->accept();

                ctx.spawn([this, client = std::move(client_sock)]() mutable -> io::task<void> {
                    try {
                        T handler{};
                        co_await handler.handle(std::move(client), settings, callback);
                    } catch (std::exception&) {}
                }());
            }
        }
        /**
         * @brief  Stop the server
         */
        io::task<void> stop() override {
            running = false;
            sock->close();
            co_return;
        }
    };
}

#endif
