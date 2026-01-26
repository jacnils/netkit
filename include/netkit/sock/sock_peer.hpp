/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file sock_peer.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides a function to get the peer address of a socket.
 */
#pragma once

#include <netkit/sock/sock_addr.hpp>

namespace netkit::sock {
    sock_addr get_peer(sock_fd_t sockfd);
}