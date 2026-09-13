/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file netkit-except.cppm
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Module partition wrapping netkit's top-level exception hierarchy (except.hpp).
 */
module;

#include <netkit/definitions.hpp>
#include <netkit/except.hpp>

export module netkit:except;

export namespace netkit {
    using namespace ::netkit;
}