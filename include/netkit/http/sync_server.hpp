/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file sync_server.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides a synchronous HTTP server class template.
 *  @see netkit::http::server::basic_sync_server
 */
#pragma once

#include <thread>
#include <netkit/sock/addr.hpp>

namespace netkit::http::server {
    /**
     * @brief  Class that represents a server.
     */
    template <typename T = request_handler<>>
    class sync_server : public basic_sync_server<> {
        bool running = true;
        server_settings settings;
        std::function<response(const request&)> callback;
        std::unique_ptr<sock::sync_sock> sock;
    public:
        /**
         * @brief  Constructor for the server class
         * @param  settings The settings for the server
         * @param  callback The function to call when a request is made
         */
        sync_server(server_settings settings, const std::function<response(const request&)>& callback)
            : settings(std::move(settings)), callback(callback)
        {
            if (!netkit::network::is_valid_port(settings.port)) {
                throw parsing_error("invalid port");
            }

            sock::addr addr = {"localhost", settings.port, netkit::sock::addr_type::hostname};
            this->sock = std::make_unique<sock::sync_sock>(addr, netkit::sock::type::tcp,
                netkit::sock::opt::reuse_addr|netkit::sock::opt::no_delay|netkit::sock::opt::blocking);

            try {
                sock->bind();
            } catch (const std::exception&) {
                throw socket_error("failed to bind socket, port might be in use");
            }
            sock->listen(settings.max_connections);
        };
        ~sync_server() override {
            sock->close();
        }
        /**
         * @brief  Run the server
         */
        void run() override {
            while (running) {
                auto client_sock = sock->accept();

                std::thread([client_sock = std::move(client_sock),
                             settings = this->settings,
                             callback = this->callback]() mutable {
                    try {
                        T handler{};
                        handler.handle(client_sock, settings, callback);
                    } catch (const std::exception& e) {
                        throw socket_error(e.what());
                    }
                }).detach();
            }
        }
        /**
         * @brief  Stop the server
         */
        void stop() override {
            running = false;
            sock->close();
        }
    };
}
