/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file local_address.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides a class representing local IP addresses (both IPv4 and IPv6) and their properties.
 */
#pragma once

#include <netkit/export.hpp>
#include <netkit/network/local_address.hpp>

#include <string>
#include <utility>

namespace netkit::network {
    class NETKIT_API local_ip_address_v4 final {
    protected:
        std::string ip{};
        std::string netmask{};
        std::string broadcast{};
        std::string peer{};
        bool loopback{};
        bool multicast{};
    public:
        local_ip_address_v4() = default;
        local_ip_address_v4(std::string ip, std::string netmask, std::string broadcast, std::string peer, bool loopback, bool multicast)
    : ip(std::move(ip)), netmask(std::move(netmask)), broadcast(std::move(broadcast)), peer(std::move(peer)), loopback(loopback), multicast(multicast) {}

        [[nodiscard]] std::string get_ip() const;
        [[nodiscard]] std::string get_netmask() const;
        [[nodiscard]] std::string get_broadcast() const;
        [[nodiscard]] std::string get_peer() const;
        [[nodiscard]] bool is_loopback() const;
        [[nodiscard]] bool is_multicast() const;
    };

    class NETKIT_API local_ip_address_v6 final {
    protected:
        std::string ip{};
        std::string netmask{};
        bool loopback{};
        bool multicast{};
        bool link_local{};
        std::string scope_id{};
    public:
        local_ip_address_v6() = default;
        local_ip_address_v6(std::string ip, std::string netmask, bool loopback, bool multicast, bool link_local, std::string scope_id)
            : ip(std::move(ip)), netmask(std::move(netmask)), loopback(loopback), multicast(multicast), link_local(link_local), scope_id(std::move(scope_id)) {}

        [[nodiscard]] std::string get_ip() const;
        [[nodiscard]] std::string get_netmask() const;
        [[nodiscard]] bool is_loopback() const;
        [[nodiscard]] bool is_multicast() const;
        [[nodiscard]] bool is_link_local() const;
        [[nodiscard]] std::string get_scope_id() const;
    };
}
