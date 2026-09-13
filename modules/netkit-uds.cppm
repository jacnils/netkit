/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file netkit-uds.cppm
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Module partition wrapping netkit's uds:: headers
 */
module;

#include <netkit/definitions.hpp>
#include <netkit/uds/async_uds_server.hpp>
#include <netkit/uds/async_uds_stream.hpp>
#include <netkit/uds/uds_server.hpp>
#include <netkit/uds/uds_stream.hpp>

export module netkit:uds;

export namespace netkit::uds {
    using namespace ::netkit::uds;
}