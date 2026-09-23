/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file types.h
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Forward declarations of every relevant handle type
 */
#pragma once

#if defined(__DEVKITPPC__) && !defined(NETKIT_C_DKP)
#define NETKIT_C_DKP 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nk_addr nk_addr_t;
typedef struct nk_socket nk_socket_t;
typedef struct nk_listener nk_listener_t;
typedef struct nk_stream nk_stream_t;
typedef struct nk_body nk_body_t;
typedef struct nk_tcp_stream nk_tcp_stream_t;
typedef struct nk_tcp_server nk_tcp_server_t;
typedef struct nk_udp_datagram nk_udp_datagram_t;
typedef struct nk_uds_stream nk_uds_stream_t;
typedef struct nk_uds_server nk_uds_server_t;
typedef struct nk_io_context nk_io_context_t;
typedef struct nk_cancellation_source nk_cancellation_source_t;
typedef struct nk_cancellation_token nk_cancellation_token_t;

#ifdef __cplusplus
}
#endif
