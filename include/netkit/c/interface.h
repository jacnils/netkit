/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file interface.h
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief C bindings for netkit::network::network_interface and netkit::network::get_interfaces()
 */
#pragma once

#include <netkit/c/common.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nk_local_ipv4 {
	char* ip;
	char* netmask;
	char* broadcast;
	char* peer;
	bool loopback;
	bool multicast;
} nk_local_ipv4_t;

typedef struct nk_local_ipv6 {
	char* ip;
	char* netmask;
	bool loopback;
	bool multicast;
	bool link_local;
	char* scope_id;
} nk_local_ipv6_t;

typedef struct nk_network_interface {
	char* name;
	nk_local_ipv4_t* ipv4;
	size_t ipv4_count;
	nk_local_ipv6_t* ipv6;
	size_t ipv6_count;
	bool up;
	bool running;
	bool broadcast;
	bool point_to_point;
} nk_network_interface_t;


NETKIT_API nk_status_t nk_network_get_interfaces(nk_network_interface_t** out, size_t* out_count);
NETKIT_API void nk_network_interfaces_free(nk_network_interface_t* interfaces, size_t count);

#ifdef __cplusplus
}
#endif
