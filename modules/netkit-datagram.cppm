/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file netkit-datagram.cppm
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Module partition wrapping netkit's datagram:: base classes
 */
module;

#include <netkit/definitions.hpp>
#include <netkit/datagram/basic_async_datagram.hpp>
#include <netkit/datagram/basic_datagram.hpp>
#include <netkit/datagram/async_socket_datagram.hpp>
#include <netkit/datagram/socket_datagram.hpp>

export module netkit:datagram;

export namespace netkit::datagram {
    using namespace ::netkit::datagram;
}