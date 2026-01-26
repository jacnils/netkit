/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file ip_list.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides a class representing a list of IP addresses (both IPv4 and IPv6).
 */
#pragma once

#include <string>
#include <netkit/export.hpp>

namespace netkit::dns {
    class dns_resolver;   // forward declaration
}

namespace netkit::sock {

    class NETKIT_API ip_list final {
    protected:
        std::string v4{};
        std::string v6{};

        friend class sync_sock;
        friend class dns::dns_resolver;

    public:
        explicit ip_list() = default;
        ip_list(const std::string& any, const std::string& second = "");

        [[nodiscard]] bool contains_ipv4() const;
        [[nodiscard]] bool contains_ipv6() const;

        [[nodiscard]] std::string get_ipv4() const;
        [[nodiscard]] std::string get_ipv6() const;
        [[nodiscard]] std::string get_ip() const;
    };
}
