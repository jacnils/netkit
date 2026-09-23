/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file tcp.h
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief C bindings for netkit::tcp::tcp_stream and netkit::tcp::tcp_server
 */
#pragma once

#include <netkit/c/common.h>
#include <netkit/c/types.h>

#ifdef __cplusplus
extern "C" {
#endif

NETKIT_API nk_status_t nk_tcp_stream_create(const nk_addr_t* addr, nk_tcp_stream_t** out);
NETKIT_API nk_status_t nk_tcp_stream_from_socket(nk_socket_t* sock, nk_tcp_stream_t** out);
NETKIT_API void nk_tcp_stream_destroy(nk_tcp_stream_t* stream);
NETKIT_API nk_status_t nk_tcp_stream_connect(nk_tcp_stream_t* stream);
NETKIT_API void nk_tcp_stream_close(nk_tcp_stream_t* stream);
NETKIT_API bool nk_tcp_stream_is_open(const nk_tcp_stream_t* stream);
NETKIT_API nk_status_t nk_tcp_stream_peer(const nk_tcp_stream_t* stream, nk_addr_t** out);
NETKIT_API nk_stream_t* nk_tcp_stream_as_stream(nk_tcp_stream_t* stream);

NETKIT_API nk_status_t nk_tcp_server_create(const nk_addr_t* addr, nk_tcp_server_t** out);
NETKIT_API void nk_tcp_server_destroy(nk_tcp_server_t* server);
NETKIT_API nk_status_t nk_tcp_server_bind(nk_tcp_server_t* server);
NETKIT_API nk_status_t nk_tcp_server_listen(nk_tcp_server_t* server, int backlog);
NETKIT_API nk_status_t nk_tcp_server_listen_default(nk_tcp_server_t* server);
NETKIT_API nk_status_t nk_tcp_server_accept(nk_tcp_server_t* server, nk_tcp_stream_t** out);
NETKIT_API void nk_tcp_server_close(nk_tcp_server_t* server);
NETKIT_API nk_status_t nk_tcp_server_get_local_endpoint(const nk_tcp_server_t* server, nk_addr_t** out);

#ifdef __cplusplus
}
#endif
