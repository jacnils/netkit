/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file netkit-socket.cppm
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Module partition wrapping netkit's socket:: headers
 */
module;

#include <netkit/definitions.hpp>
#include <netkit/platform/socket.hpp>
#include <netkit/socket/addr.hpp>
#include <netkit/socket/addr_type.hpp>
#include <netkit/socket/native/native_sync_socket.hpp>
#include <netkit/socket/native/basic_native_async_socket.hpp>
#include <netkit/socket/native/native_async_socket.hpp>
#include <netkit/socket/native/native_sync_listener.hpp>
#include <netkit/socket/native/native_async_listener.hpp>
#include <netkit/socket/native/peer_helper.hpp>

export module netkit:socket;

export namespace netkit::socket {
    using namespace ::netkit::socket;
}

export namespace netkit::socket::native {
    using namespace ::netkit::socket::native;
}