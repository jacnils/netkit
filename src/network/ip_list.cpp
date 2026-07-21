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
#include "netkit/except.hpp"

#include <netkit/network/ip_list.hpp>
#include <netkit/network/utility.hpp>

netkit::network::ip_list::ip_list(const std::string& any, const std::string& second) {
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

bool netkit::network::ip_list::contains_ipv4() const {
    return !this->v4.empty();
}

bool netkit::network::ip_list::contains_ipv6() const {
    return !this->v6.empty();
}

std::string netkit::network::ip_list::get_ipv4() const {
    return this->v4;
}

std::string netkit::network::ip_list::get_ipv6() const {
    return this->v6;
}

std::string netkit::network::ip_list::get_ip() const {
    return this->v6.empty() ? this->v4 : this->v6;
}

void netkit::network::ip_list::set_ipv4(const std::string& ip) {
	if (netkit::network::is_ipv4(ip) == false) {
		throw ip_error{"invalid ipv4 address"};
	}
	this->v4 = ip;
}
void netkit::network::ip_list::set_ipv6(const std::string& ip) {
	if (netkit::network::is_ipv6(ip) == false) {
		throw ip_error{"invalid ipv6 address"};
	}
	this->v6 = ip;
}
void netkit::network::ip_list::set_ip(const std::string& ip) {
	if (netkit::network::is_ipv4(ip) && !ip.empty()) {
		this->v4 = ip;
	} else if (netkit::network::is_ipv6(ip) && !ip.empty()) {
		this->v6 = ip;
	}
}