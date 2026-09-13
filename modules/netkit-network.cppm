/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file netkit-network.cppm
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Module partition wrapping netkit's network:: headers
 */
module;

#include <netkit/definitions.hpp>
#include <netkit/network/utility.hpp>
#include <netkit/network/local_address.hpp>
#include <netkit/network/network_interface.hpp>
#include <netkit/network/ip_list.hpp>

export module netkit:network;

export namespace netkit::network {
    using namespace ::netkit::network;
}