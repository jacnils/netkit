/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file local_address.cpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Implementation of the local IP address classes.
 */
#include <netkit/network/local_address.hpp>

#include <string>

std::string netkit::network::local_ip_address_v4::get_ip() const {
    return this->ip;
}

std::string netkit::network::local_ip_address_v4::get_netmask() const {
    return this->netmask;
}

std::string netkit::network::local_ip_address_v4::get_broadcast() const {
    return this->broadcast;
}

std::string netkit::network::local_ip_address_v4::get_peer() const {
    return this->peer;
}

bool netkit::network::local_ip_address_v4::is_loopback() const {
    return this->loopback;
}

bool netkit::network::local_ip_address_v4::is_multicast() const {
    return this->multicast;
}

std::string netkit::network::local_ip_address_v6::get_ip() const {
    return this->ip;
}

std::string netkit::network::local_ip_address_v6::get_netmask() const {
    return this->netmask;
}

bool netkit::network::local_ip_address_v6::is_loopback() const {
    return this->loopback;
}

bool netkit::network::local_ip_address_v6::is_multicast() const {
    return this->multicast;
}

bool netkit::network::local_ip_address_v6::is_link_local() const {
    return this->link_local;
}

std::string netkit::network::local_ip_address_v6::get_scope_id() const {
    return this->scope_id;
}