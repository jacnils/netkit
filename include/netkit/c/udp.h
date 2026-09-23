/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file udp.h
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief C bindings for netkit::udp::udp_datagram.
 */
#pragma once

#include <netkit/c/common.h>
#include <netkit/c/types.h>

#ifdef __cplusplus
extern "C" {
#endif

NETKIT_API nk_status_t nk_udp_datagram_create(const nk_addr_t* addr, nk_udp_datagram_t** out);
NETKIT_API void nk_udp_datagram_destroy(nk_udp_datagram_t* dgram);
NETKIT_API nk_status_t nk_udp_datagram_bind(nk_udp_datagram_t* dgram);
NETKIT_API nk_status_t nk_udp_datagram_send_to(nk_udp_datagram_t* dgram, const void* buffer, size_t len, const nk_addr_t* dest, size_t* out_sent);
NETKIT_API nk_status_t nk_udp_datagram_recv_from(nk_udp_datagram_t* dgram, void* buffer, size_t len, size_t* out_received, nk_addr_t** out_from);
NETKIT_API void nk_udp_datagram_close(nk_udp_datagram_t* dgram);
NETKIT_API bool nk_udp_datagram_is_open(const nk_udp_datagram_t* dgram);

#ifdef __cplusplus
}
#endif
