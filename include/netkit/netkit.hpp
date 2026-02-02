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
#include <netkit/sock/sock_addr_type.hpp>
#include <netkit/sock/sock_addr.hpp>
#include <netkit/sock/sock_peer.hpp>
#include <netkit/sock/sync_sock.hpp>
#include <netkit/sock/openssl/ssl_sync_sock.hpp>

// HTTP headers
#include <netkit/http/predefined.hpp>
#include <netkit/http/response.hpp>
#include <netkit/http/body_parser.hpp>
#include <netkit/http/sync_client.hpp>
#include <netkit/http/basic_request_handler.hpp>
#include <netkit/http/request_handler.hpp>
#include <netkit/http/server_predefined.hpp>
#include <netkit/http/basic_sync_server.hpp>
#include <netkit/http/sync_server.hpp>
#include <netkit/http/sync_client.hpp>

#pragma message ("Use of netkit.hpp directly is discouraged for all uses, except test code.")