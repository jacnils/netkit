/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file netkit-body.cppm
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Module partition wrapping netkit's body:: headers (basic_body/basic_async_body and implementations).
 */
module;

#include <netkit/definitions.hpp>
#include <netkit/body/basic_body.hpp>
#include <netkit/body/file_body.hpp>
#include <netkit/body/buffer_body.hpp>
#include <netkit/body/buffer_body_view.hpp>
#include <netkit/body/stream_body.hpp>
#include <netkit/body/multipart_part_body.hpp>
#include <netkit/body/basic_async_body.hpp>
#include <netkit/body/async_stream_body.hpp>
#include <netkit/body/read_status_enum.hpp>
#include <netkit/body/async_multipart_part_body.hpp>
#include <netkit/body/async_buffer_body.hpp>
#include <netkit/body/async_file_body.hpp>
#include <netkit/body/async_buffer_body_view.hpp>
#include <netkit/body/async_chunked_body.hpp>
#include <netkit/body/chunked_body.hpp>

export module netkit:body;

export namespace netkit::body {
    using namespace ::netkit::body;
}