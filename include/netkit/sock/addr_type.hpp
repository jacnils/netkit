/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file addr_type.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides common socket types, options, and related enums and structs.
 */
#pragma once

#include <netkit/network/ip_list.hpp>
#include <netkit/definitions.hpp>

#ifdef NETKIT_WINDOWS
#include <winsock2.h>
#endif

namespace netkit::sock {
    /**
     * @brief Socket file descriptor type.
     * @note This is a typedef for int, but can be changed to a different type if needed.
     */
#ifdef NETKIT_WINDOWS
    using fd_t = SOCKET;
#elifdef NETKIT_UNIX
    using fd_t = int;
#endif

    enum class addr_type {
        ipv4 = 0, /* IPv4 address */
        ipv6 = 1, /* IPv6 address */
        hostname_ipv4 = 2, /* Hostname; resolve to IPv4 address */
        hostname_ipv6 = 3, /* Hostname; resolve to IPv6 address */
        hostname = 4, /* Hostname; resolve to IPv4 address */
        filename = 5 /* File path; used for Unix domain sockets */
    };

    /**
     * @brief Socket types.
     */
    enum class type {
        tcp, /* TCP socket */
        udp, /* UDP socket */
        unix, /* UNIX domain socket */
    };
    /**
     * @brief Socket options.
     * @note These options can be used with the sync_sock class to set socket options.
     */
    enum class opt {
        reuse_addr = 1 << 0, /* Reuse address option */
        no_reuse_addr = 1 << 1, /* Do not reuse address option */
        no_delay = 1 << 2, /* Disable Nagle's algorithm (TCP_NODELAY) */
        keep_alive = 1 << 3, /* Enable keep-alive option */
        no_keep_alive = 1 << 4, /* Disable keep-alive option */
        no_blocking = 1 << 5, /* Set socket to non-blocking mode. Not necessarily supported. */
        blocking = 1 << 6, /* Set socket to blocking mode */
    };

    /**
     * @brief Socket receive status.
     * @note This enum is used to indicate the status of a socket receive operation.
     */
    enum class recv_status {
        success,
        timeout,
        closed,
        error
    };

    /**
     * @brief Result of a socket receive operation.
     * @note This struct contains the result data and the status of the receive operation.
     */
    struct recv_result {
        std::string data{};
        recv_status status{recv_status::success};
    };

    inline opt operator|(opt lhs, opt rhs) {
        using T = std::underlying_type_t<opt>;
        return static_cast<opt>(static_cast<T>(lhs) | static_cast<T>(rhs));
    }

    inline bool operator&(opt lhs, opt rhs) {
        using T = std::underlying_type_t<opt>;
        return static_cast<T>(lhs) & static_cast<T>(rhs);
    }
}