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

// Socket headers
#include <netkit/socket/addr.hpp>
#include <netkit/socket/addr_type.hpp>
#include <netkit/socket/native_sync_sock.hpp>
#include <netkit/socket/openssl/ssl_sync_sock.hpp>
#include <netkit/socket/sock_peer.hpp>
#include <netkit/socket/ssl_sync_sock.hpp>
#include <netkit/socket/ssl_sync_sock_enum.hpp>
#include <netkit/socket/wolfssl/ssl_sync_sock.hpp>
#include <socket/native/basic_native_async_sock.hpp>
#include <socket/native/native_async_sock.hpp>

// HTTP headers
#include <netkit/http/basic_request_handler.hpp>
#include <netkit/http/basic_sync_server.hpp>
#include <netkit/http/body_parser.hpp>
#include <netkit/http/multipart.hpp>
#include <netkit/http/multipart_reader.hpp>
#include <netkit/http/predefined.hpp>
#include <netkit/http/request_handler.hpp>
#include <netkit/http/response.hpp>
#include <netkit/http/server_predefined.hpp>
#include <netkit/http/sync_client.hpp>
#include <netkit/http/sync_server.hpp>

// bodies
#include <netkit/body/basic_body.hpp>
#include <netkit/body/file_body.hpp>
#include <netkit/body/buffer_body.hpp>
#include <netkit/body/buffer_body_view.hpp>
#include <netkit/body/stream_body.hpp>
#include <netkit/body/multipart_part_body.hpp>

// io
#include <netkit/io/basic_io_context.hpp>
#include <netkit/io/io_awaitable.hpp>
#include <netkit/io/io_context.hpp>
#include <netkit/io/task.hpp>

#pragma message ("Use of netkit.hpp directly is discouraged for all uses, except test code.")