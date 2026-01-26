/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file ip_list.cpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Implementation of the IP address list class.
 */
#include <netkit/sock/ip_list.hpp>
#include <netkit/network/utility.hpp>

netkit::sock::ip_list::ip_list(const std::string& any, const std::string& second) {
    if (netkit::network::is_ipv4(any) && !any.empty()) {
        this->v4 = any;
    } else if (netkit::network::is_ipv6(any) && !any.empty()) {
        this->v6 = any;
    }
    if (netkit::network::is_ipv4(second) && !second.empty()) {
        this->v4 = second;
    } else if (netkit::network::is_ipv6(second) && !second.empty()) {
        this->v6 = second;
    }
}

bool netkit::sock::ip_list::contains_ipv4() const {
    return !this->v4.empty();
}

bool netkit::sock::ip_list::contains_ipv6() const {
    return !this->v6.empty();
}

std::string netkit::sock::ip_list::get_ipv4() const {
    return this->v4;
}

std::string netkit::sock::ip_list::get_ipv6() const {
    return this->v6;
}

std::string netkit::sock::ip_list::get_ip() const {
    return this->v6.empty() ? this->v4 : this->v6;
}