/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file utility.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides network utility functions, such as IP address validation and port validation.
 */
#pragma once

#include <string>

#include <netkit/export.hpp>

namespace netkit::network {
    /**
     * @brief A function that checks if a usable IPv6 address exists.
     * @return True if a usable IPv6 address exists, false otherwise.
     * @note This function checks the local network interfaces for a usable IPv6 address.
     */
    NETKIT_API bool usable_ipv6_address_exists();
    /**
     * @brief A function that checks if a string is a valid IPv4 address.
     * @param ip The string to check.
     * @return True if the string is a valid IPv4 address, false otherwise.
     */
    NETKIT_API bool is_ipv4(const std::string& ip);
    /**
     * @brief A function that checks if a string is a valid IPv6 address.
     * @param ip The string to check.
     * @return True if the string is a valid IPv6 address, false otherwise.
     */
    NETKIT_API bool is_ipv6(const std::string& ip);
    /**
     * @brief A function that checks if a port is valid.
     * @param port The port to check.
     * @return True if the port is valid, false otherwise.
     */
    NETKIT_API bool is_valid_port(int port);
}