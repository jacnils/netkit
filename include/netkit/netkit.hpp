/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file netkit.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @note External use of this header is not recommended; prefer including only the specific headers you need.
 *  @brief Main include file for the Netkit library, aggregating all headers.
 */
#pragma once

#define NETKIT_NETKIT

// Essential headers
#include <netkit/definitions.hpp>
#include <netkit/except.hpp>
#include <netkit/export.hpp>
#include <netkit/utility.hpp>

// Networking headers
#include <netkit/network/utility.hpp>
#include <netkit/network/local_address.hpp>
#include <netkit/network/network_interface.hpp>
#include <netkit/network/ip_list.hpp>

// DNS headers
#include <netkit/dns/record_type.hpp>
#include <netkit/dns/sync_resolver.hpp>
#include <netkit/dns/nameserver_list.hpp>
#include <netkit/dns/cache.hpp>
#include <netkit/dns/response_parser.hpp>
#include <netkit/dns/query_builder.hpp>

// Platform headers
#include <netkit/platform/socket.hpp>

// Socket headers
#include <netkit/socket/addr.hpp>
#include <netkit/socket/addr_type.hpp>
#include <netkit/socket/native/native_sync_socket.hpp>
#include <netkit/socket/native/basic_native_async_socket.hpp>
#include <netkit/socket/native/native_async_socket.hpp>
#include <netkit/socket/native/peer_helper.hpp>

// HTTP headers
#include <netkit/http/basic_request_handler.hpp>
#include <netkit/http/basic_sync_server.hpp>
#include <netkit/http/multipart.hpp>
#include <netkit/http/multipart_reader.hpp>
#include <netkit/http/multipart_reader_state.hpp>
#include <netkit/http/async_multipart_reader.hpp>
#include <netkit/http/predefined.hpp>
#include <netkit/http/request_handler.hpp>
#include <netkit/http/server_predefined.hpp>
#include <netkit/http/sync_server.hpp>
#include <netkit/http/basic_async_server.hpp>
#include <netkit/http/async_server.hpp>
#include <netkit/http/client.hpp>
#include <netkit/http/header.hpp>
#include <netkit/http/async_client.hpp>

// bodies
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

// io
#include <netkit/io/linux/io_backend.hpp>
#include <netkit/io/fallback/io_backend.hpp>
#include <netkit/io/windows/io_backend.hpp>
#include <netkit/io/bsd/io_backend.hpp>
#include <netkit/io/io_backend.hpp>
#include <netkit/io/io_awaitable.hpp>
#include <netkit/io/io_context.hpp>
#include <netkit/io/task.hpp>
#include <netkit/io/cancellation.hpp>
#include <netkit/io/timeout.hpp>

// streams
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

// tcp
#include <netkit/tcp/async_tcp_server.hpp>
#include <netkit/tcp/async_tcp_stream.hpp>
#include <netkit/tcp/tcp_server.hpp>
#include <netkit/tcp/tcp_stream.hpp>

// datagram
#include <netkit/datagram/basic_async_datagram.hpp>
#include <netkit/datagram/basic_datagram.hpp>
#include <netkit/datagram/async_socket_datagram.hpp>
#include <netkit/datagram/socket_datagram.hpp>

// udp
#include <netkit/udp/async_udp_datagram.hpp>
#include <netkit/udp/udp_datagram.hpp>

// uds
#include <netkit/uds/async_uds_server.hpp>
#include <netkit/uds/async_uds_stream.hpp>
#include <netkit/uds/uds_server.hpp>
#include <netkit/uds/uds_stream.hpp>

#pragma message ("Use of netkit.hpp directly is discouraged for all uses, except test code.")