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

#include <netkit/sock/sock_peer.hpp>
#include <netkit/sock/sock_addr.hpp>
#include <netkit/except.hpp>
#include <netkit/definitions.hpp>
#ifdef NETKIT_UNIX
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#elifdef NETKIT_WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

netkit::sock::sock_addr netkit::sock::get_peer(sock_fd_t sockfd) {
    sockaddr_storage addr_storage{};
    socklen_t addr_len = sizeof(addr_storage);

    if (getpeername(sockfd, reinterpret_cast<sockaddr*>(&addr_storage), &addr_len) < 0) {
        throw socket_error("getpeername() failed: " + std::string(strerror(errno)));
    }

    char ip_str[INET6_ADDRSTRLEN] = {0};
    uint16_t port = 0;

    if (addr_storage.ss_family == AF_INET) {
        auto* addr_in = reinterpret_cast<sockaddr_in*>(&addr_storage);
        inet_ntop(AF_INET, &(addr_in->sin_addr), ip_str, sizeof(ip_str));
        port = ntohs(addr_in->sin_port);
    } else if (addr_storage.ss_family == AF_INET6) {
        auto* addr_in6 = reinterpret_cast<sockaddr_in6*>(&addr_storage);
        inet_ntop(AF_INET6, &(addr_in6->sin6_addr), ip_str, sizeof(ip_str));
        port = ntohs(addr_in6->sin6_port);
    } else {
        throw ip_error("unsupported address family");
    }

    sock_addr addr{};
    addr.ip = ip_str;
    addr.port = port;
    addr.type = (addr_storage.ss_family == AF_INET) ?
        sock_addr_type::ipv4 : sock_addr_type::ipv6;

    return addr;
}
