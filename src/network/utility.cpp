/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file utility.cpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Implementation of network utility functions.
 */
#include <cstdint>
#include <netkit/network/utility.hpp>
#include <netkit/network/network_interface.hpp>
#include <netkit/definitions.hpp>

#ifdef NETKIT_WINDOWS
#include <ws2tcpip.h>
#elif NETKIT_UNIX
#include <arpa/inet.h>
#endif

bool netkit::network::is_ipv4(const std::string& ip) {
    sockaddr_in sa{};
    return inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr)) != 0;
}

bool netkit::network::is_ipv6(const std::string &ip) {
    sockaddr_in6 sa{};
    return inet_pton(AF_INET6, ip.c_str(), &(sa.sin6_addr)) != 0;
}

bool netkit::network::is_valid_port(int port) {
    return port > 0 && port <= 65535;
}

bool netkit::network::usable_ipv6_address_exists() {
    static auto interfaces = get_interfaces();

    for (const auto& iface : interfaces) {
        if (!iface.is_up()) continue;

        for (const auto& addr : iface.get_ipv6_addrs()) {
            const auto& ip = addr.get_ip();
            if (ip.empty()) continue;

            if (addr.is_loopback() || addr.is_link_local() || addr.is_multicast()) {
                continue;
            }

            return true;
        }
    }

    return false;
}
