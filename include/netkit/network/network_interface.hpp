/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file network_interface.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides a class allowing retrieval of local network interfaces and their properties.
 */
#pragma once

#include <vector>

#include <netkit/export.hpp>
#include <netkit/network/local_address.hpp>

namespace netkit::network {
    class NETKIT_API network_interface final {
    protected:
        std::vector<local_ip_address_v4> ipv4{};
        std::vector<local_ip_address_v6> ipv6{};
        std::string name{};
        bool up{false};
        bool running{false};
        bool broadcast{false};
        bool point_to_point{false};

        friend std::vector<network_interface> get_interfaces();
    public:
        network_interface() = default;

        [[nodiscard]] const std::vector<local_ip_address_v4>& get_ipv4_addrs() const;
        [[nodiscard]] const std::vector<local_ip_address_v6>& get_ipv6_addrs() const;
        [[nodiscard]] std::string get_name() const;
        [[nodiscard]] bool is_up() const;
        [[nodiscard]] bool is_running() const;
        [[nodiscard]] bool is_broadcast() const;
        [[nodiscard]] bool is_point_to_point() const;

        void set_ipv4_addrs(const std::vector<local_ip_address_v4>& ipv4_addrs);
        void set_ipv6_addrs(const std::vector<local_ip_address_v6>& ipv6_addrs);
        void set_name(const std::string& name);
        void set_up(bool value);
        void set_running(bool value);
        void set_broadcast(bool value);
        void set_point_to_point(bool value);
    };

    /**
     * @brief A function that gets the local network interfaces.
     * @return A vector of network_interface structs that contain the local network interfaces.
     */
    std::vector<network_interface> get_interfaces();
}