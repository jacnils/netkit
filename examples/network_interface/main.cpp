/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file main.cpp
 *  @license MIT
 *  @note Example code using the Netkit library.
 *  @brief A clone of the 'ifconfig' command that lists network interfaces and their addresses.
 */
#include <iostream>
#include <unistd.h>
#include <netkit/netkit.hpp>

const std::string RESET = "\033[0m";
const std::string BOLD = "\033[1m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string CYAN = "\033[36m";
const std::string MAGENTA = "\033[35m";

int main(int argc, char** argv) {
    bool use_color = true;
    bool skip_v4 = false;
    bool skip_v6 = false;
    bool skip_inactive = false;
    bool skip_broadcast = false;
    bool skip_point_to_point = false;
    bool skip_up = false;
    bool skip_loopback = false;
    bool skip_multicast = false;
    bool skip_no_ipv4 = false;
    bool skip_no_ipv6 = false;
    bool skip_loopback_interfaces = false;

    bool is_stdout_terminal = isatty(STDOUT_FILENO);
    if (!is_stdout_terminal) {
        use_color = false;
    }

    static const auto args = std::vector<std::string>(argv, argv + argc);
    for (const auto& it : args) {
        if (it == "--no-color" || it == "-n") {
            use_color = false;
        } else if (it == "--color" || it == "-c") {
            use_color = true;
        } else if (it == "--skip-v4" || it == "-s4") {
            skip_v4 = true;
        } else if (it == "--skip-v6" || it == "-s6") {
            skip_v6 = true;
        } else if (it == "--skip-inactive" || it == "-si") {
            skip_inactive = true;
        } else if (it == "--skip-broadcast" || it == "-sb") {
            skip_broadcast = true;
        } else if (it == "--skip-point-to-point" || it == "-sptp") {
            skip_point_to_point = true;
        } else if (it == "--skip-up" || it == "-su") {
            skip_up = true;
        } else if (it == "--skip-loopback" || it == "-sl") {
            skip_loopback = true;
        } else if (it == "--skip-no-addresses" || it == "-sna") {
            skip_no_ipv4 = true;
            skip_no_ipv6 = true;
        } else if (it == "--skip-no-ipv4" || it == "-snv4") {
            skip_no_ipv4 = true;
        } else if (it == "--skip-no-ipv6" || it == "-snv6") {
            skip_no_ipv6 = true;
        } else if (it == "--skip-loopback-interfaces" || it == "-sli") {
            skip_loopback_interfaces = true;
        } else if (it == "--help" || it == "-h") {
            std::cout << "Usage: " << argv[0] << " [options]\n"
                      << "Options:\n"
                      << "  --no-color, -n               Disable colored output\n"
                      << "  --color, -c                  Enable colored output (default)\n"
                      << "  --skip-v4, -s4               Skip IPv4 addresses\n"
                      << "  --skip-v6, -s6               Skip IPv6 addresses\n"
                      << "  --skip-inactive, -si         Skip inactive interfaces\n"
                      << "  --skip-broadcast, -sb        Skip broadcast interfaces\n"
                      << "  --skip-point-to-point, -sptp Skip point-to-point interfaces\n"
                      << "  --skip-up, -su               Skip interfaces that are not up\n"
                      << "  --skip-loopback, -sl         Skip loopback addresses\n"
                      << "  --skip-multicast, -sm        Skip multicast addresses\n"
                      << "  --skip-no-addresses, -sna    Skip interfaces with no IPv4 or IPv6 addresses\n"
                      << "  --skip-no-ipv4, -snv4        Skip interfaces with no IPv4 addresses\n"
                      << "  --skip-no-ipv6, -snv6        Skip interfaces with no IPv6 addresses\n"
                      << "  --skip-loopback-interfaces, -sli Skip loopback interfaces\n"
                      << "  --help, -h                   Show this help message\n";
            return 0;
        }
    }

    auto list = netkit::network::get_interfaces();

    for (const auto& iface : list) {
        if (skip_inactive && !iface.is_up()) continue;
        if (skip_broadcast && iface.is_broadcast()) continue;
        if (skip_point_to_point && iface.is_point_to_point()) continue;
        if (skip_up && !iface.is_up()) continue;
        if (iface.get_ipv4_addrs().empty() && skip_no_ipv4) continue;
        if (iface.get_ipv6_addrs().empty() && skip_no_ipv6) continue;
        if (skip_loopback_interfaces) {
            bool found = false;
            for (const auto& addr : iface.get_ipv4_addrs()) {
                if (addr.is_loopback()) found = true; break;
            }
            for (const auto& addr : iface.get_ipv6_addrs()) {
                if (addr.is_loopback()) found = true; break;
            }
            if (found) {
                continue;
            }
        }

        if (use_color)
            std::cout << BOLD << CYAN;
        std::cout << "Interface: " << iface.get_name();
        if (use_color)
            std::cout << RESET;
        std::cout << "\n";

        iface.is_broadcast() ? std::cout << "  Broadcast: " << (use_color ? GREEN : "") << "Yes" << (use_color ? RESET : "") << "\n"
                             : std::cout << "  Broadcast: No\n";
        iface.is_point_to_point() ? std::cout << "  Point-to-Point: " << (use_color ? GREEN : "") << "Yes" << (use_color ? RESET : "") << "\n"
                                  : std::cout << "  Point-to-Point: No\n";
        iface.is_up() ? std::cout << "  Up: " << (use_color ? GREEN : "") << "Yes" << (use_color ? RESET : "") << "\n"
                      : std::cout << "  Up: No\n";
        iface.is_running() ? std::cout << "  Running: " << (use_color ? GREEN : "") << "Yes" << (use_color ? RESET : "") << "\n"
                           : std::cout << "  Running: No\n";

        if (!skip_v4) std::cout << "  IPv4 Addresses:\n";
        if (iface.get_ipv4_addrs().empty() && !skip_v4) {
            std::cout << "    No IPv4 addresses found.\n";
        }
        for (const auto& addr : iface.get_ipv4_addrs()) {
            if (skip_inactive && !iface.is_up()) continue;
            if (skip_v4) continue;
            if (skip_loopback && addr.is_loopback()) continue;
            if (skip_multicast && addr.is_multicast()) continue;
            if (skip_broadcast && addr.get_broadcast().empty()) continue;

            if (use_color)
                std::cout << YELLOW;
            std::cout << "    " << addr.get_ip();
            if (use_color)
                std::cout << RESET;
            std::cout << "\n";

            if (addr.is_multicast())
                std::cout << "      (Multicast)\n";
            if (addr.is_loopback())
                std::cout << "      (Loopback)\n";
            if (!addr.get_broadcast().empty())
                std::cout << "      Broadcast: " << addr.get_broadcast() << "\n";
            if (!addr.get_netmask().empty())
                std::cout << "      Netmask: " << addr.get_netmask() << "\n";
            if (!addr.get_peer().empty())
                std::cout << "      Peer: " << addr.get_peer() << "\n";
        }
        if (!skip_v6) std::cout << "  IPv6 Addresses:\n";
        if (iface.get_ipv6_addrs().empty() && !skip_v6) {
            std::cout << "    No IPv6 addresses found.\n";
        }
        for (const auto& addr : iface.get_ipv6_addrs()) {
            if (skip_v6) continue;
            if (skip_loopback && addr.is_loopback()) continue;
            if (skip_multicast && addr.is_multicast()) continue;
            if (skip_inactive && !iface.is_up()) continue;
            if (skip_broadcast && addr.get_netmask().empty()) continue;

            if (use_color)
                std::cout << MAGENTA;
            std::cout << "    " << addr.get_ip();
            if (use_color)
                std::cout << RESET;
            std::cout << "\n";

            if (addr.is_link_local())
                std::cout << "      (Link Local)\n";
            if (addr.is_multicast())
                std::cout << "      (Multicast)\n";
            if (addr.is_loopback())
                std::cout << "      (Loopback)\n";
            if (!addr.get_scope_id().empty())
                std::cout << "      Scope ID: " << addr.get_scope_id() << "\n";
            if (!addr.get_netmask().empty())
                std::cout << "      Netmask: " << addr.get_netmask() << "\n";
        }
    }
}
