/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file netkit-udp.cppm
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Module partition wrapping netkit's udp:: headers
 */
module;

#include <netkit/definitions.hpp>
#include <netkit/udp/async_udp_datagram.hpp>
#include <netkit/udp/udp_datagram.hpp>

export module netkit:udp;

export namespace netkit::udp {
    using namespace ::netkit::udp;
}