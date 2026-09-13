/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file netkit-tcp.cppm
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Module partition wrapping netkit's tcp:: headers
 */
module;

#include <netkit/definitions.hpp>
#include <netkit/tcp/async_tcp_server.hpp>
#include <netkit/tcp/async_tcp_stream.hpp>
#include <netkit/tcp/tcp_server.hpp>
#include <netkit/tcp/tcp_stream.hpp>

export module netkit:tcp;

export namespace netkit::tcp {
    using namespace ::netkit::tcp;
}