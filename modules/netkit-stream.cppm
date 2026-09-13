/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file netkit-stream.cppm
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Module partition wrapping netkit's stream:: headers
 */
module;

#include <netkit/definitions.hpp>
#include <netkit/stream/stream_enum.hpp>
#include <netkit/stream/basic_stream.hpp>
#include <netkit/stream/memory_stream.hpp>
#include <netkit/stream/socket_stream.hpp>
#include <netkit/stream/basic_async_stream.hpp>
#include <netkit/stream/async_socket_stream.hpp>
#include <netkit/stream/wolfssl/tls_stream.hpp>
#include <netkit/stream/tls_stream.hpp>
#include <netkit/stream/tls_stream_enum.hpp>
#include <netkit/stream/utility.hpp>
#include <netkit/stream/wolfssl/async_tls_stream.hpp>
#include <netkit/stream/async_tls_stream.hpp>

export module netkit:stream;

export namespace netkit::stream {
    using namespace ::netkit::stream;
}