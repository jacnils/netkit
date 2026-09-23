/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file uds.h
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief C bindings for netkit::uds::uds_stream and netkit::uds::uds_server
 */
#pragma once

#include <netkit/c/common.h>
#include <netkit/c/types.h>

#ifndef NETKIT_C_DKP

#ifdef __cplusplus
extern "C" {
#endif

NETKIT_API nk_status_t nk_uds_stream_create(const nk_addr_t* addr, nk_uds_stream_t** out);
NETKIT_API nk_status_t nk_uds_stream_from_socket(nk_socket_t* sock, nk_uds_stream_t** out);
NETKIT_API void nk_uds_stream_destroy(nk_uds_stream_t* stream);
NETKIT_API nk_status_t nk_uds_stream_connect(nk_uds_stream_t* stream);
NETKIT_API void nk_uds_stream_close(nk_uds_stream_t* stream);
NETKIT_API bool nk_uds_stream_is_open(const nk_uds_stream_t* stream);
NETKIT_API nk_status_t nk_uds_stream_peer(const nk_uds_stream_t* stream, nk_addr_t** out);
NETKIT_API nk_stream_t* nk_uds_stream_as_stream(nk_uds_stream_t* stream);
NETKIT_API nk_status_t nk_uds_server_create(const nk_addr_t* addr, nk_uds_server_t** out);
NETKIT_API void nk_uds_server_destroy(nk_uds_server_t* server);
NETKIT_API nk_status_t nk_uds_server_bind(nk_uds_server_t* server);
NETKIT_API nk_status_t nk_uds_server_listen(nk_uds_server_t* server, int backlog);
NETKIT_API nk_status_t nk_uds_server_listen_default(nk_uds_server_t* server);
NETKIT_API nk_status_t nk_uds_server_accept(nk_uds_server_t* server, nk_uds_stream_t** out);
NETKIT_API void nk_uds_server_close(nk_uds_server_t* server);
NETKIT_API nk_status_t nk_uds_server_get_local_endpoint(const nk_uds_server_t* server, nk_addr_t** out);

#ifdef __cplusplus
}
#endif

#endif /* !NETKIT_C_DKP */
