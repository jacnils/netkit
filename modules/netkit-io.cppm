/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file netkit-io.cppm
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Module partition wrapping netkit's io:: headers
 */
module;

#include <netkit/definitions.hpp>
#include <netkit/io/linux/io_backend.hpp>
#include <netkit/io/fallback/io_backend.hpp>
#include <netkit/io/windows/io_backend.hpp>
#include <netkit/io/bsd/io_backend.hpp>
#include <netkit/io/io_backend.hpp>
#include <netkit/io/basic_io_backend.hpp>
#include <netkit/io/io_awaitable.hpp>
#include <netkit/io/io_context.hpp>
#include <netkit/io/task.hpp>
#include <netkit/io/cancellation.hpp>
#include <netkit/io/timeout.hpp>

export module netkit:io;

export namespace netkit::io {
    using namespace ::netkit::io;
}