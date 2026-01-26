/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file basic_request_handler.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides a basic HTTP server request handler interface.
 */
#pragma once

#include <netkit/http/server_predefined.hpp>
#include <netkit/sock/basic_sync_sock.hpp>

namespace netkit::http::server {
    template <typename S = server_settings>
    class basic_request_handler {
    public:
        virtual void handle(std::unique_ptr<netkit::sock::basic_sync_sock>&, server_settings&, const request_callback&) const = 0;
        virtual ~basic_request_handler() = default;
    };
}
