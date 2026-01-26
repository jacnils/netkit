/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file basic_sync_server.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides an interface class that represents a basic synchronous HTTP server.
 */
#pragma once

#include <netkit/http/basic_request_handler.hpp>
#include <netkit/http/request_handler.hpp>
#include <netkit/sock/sync_sock.hpp>
#include <netkit/network/utility.hpp>

namespace netkit::http::server {
    /**
     * @brief  Interface class that represents a server.
     */
    template <typename T = request_handler<>>
    class basic_sync_server {
    public:
        virtual ~basic_sync_server() = default;
        virtual void run() = 0;
        virtual void stop() = 0;
    };
}
