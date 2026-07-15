/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file sock_peer.cpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Implementation of the function to get the peer address of a socket.
 */
#include <cstring>
#include <netkit/definitions.hpp>
#include <netkit/except.hpp>
#include <netkit/sock/addr.hpp>
#include <netkit/sock/sock_peer.hpp>
#ifdef NETKIT_UNIX
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#elifdef NETKIT_WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#endif