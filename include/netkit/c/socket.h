/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file socket.h
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief C bindings for netkit::socket::addr, netkit::socket::native::native_sync_socket and netkit::socket::native::native_sync_listener.
 */
#pragma once

#include <netkit/c/common.h>
#include <netkit/c/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum nk_addr_type {
	NK_ADDR_IPV4 = 0,
	NK_ADDR_IPV6 = 1,
	NK_ADDR_HOSTNAME_IPV4 = 2,
	NK_ADDR_HOSTNAME_IPV6 = 3,
	NK_ADDR_HOSTNAME = 4,
	NK_ADDR_FILENAME = 5
} nk_addr_type_t;

typedef enum nk_resolve_method {
	NK_RESOLVE_OS = 0,
	NK_RESOLVE_NETKIT = 1,
	NK_RESOLVE_DOT = 2
} nk_resolve_method_t;

typedef enum nk_socket_type {
	NK_SOCK_TCP = 0,
	NK_SOCK_UDP = 1,
	NK_SOCK_UDS = 2
} nk_socket_type_t;

typedef enum nk_socket_opt {
	NK_OPT_REUSE_ADDR = 1 << 0,
	NK_OPT_NO_REUSE_ADDR = 1 << 1,
	NK_OPT_NO_DELAY = 1 << 2,
	NK_OPT_KEEP_ALIVE = 1 << 3,
	NK_OPT_NO_KEEP_ALIVE = 1 << 4,
	NK_OPT_NO_BLOCKING = 1 << 5,
	NK_OPT_BLOCKING = 1 << 6
} nk_socket_opt_t;

// to be honest i don't remember what the rationale was for these defaults
#define NK_OPT_DEFAULT ((uint32_t) (NK_OPT_REUSE_ADDR | NK_OPT_NO_DELAY | NK_OPT_BLOCKING))

NETKIT_API nk_status_t nk_addr_create(const char* hostname, int port, nk_addr_type_t type, nk_resolve_method_t method, nk_addr_t** out);

#ifndef NETKIT_C_DKP
NETKIT_API nk_status_t nk_addr_create_path(const char* path, nk_addr_t** out);
#endif

NETKIT_API nk_addr_t* nk_addr_clone(const nk_addr_t* addr);

NETKIT_API void nk_addr_destroy(nk_addr_t* addr);

NETKIT_API bool nk_addr_is_ipv4(const nk_addr_t* addr);
NETKIT_API bool nk_addr_is_ipv6(const nk_addr_t* addr);
NETKIT_API bool nk_addr_is_file_path(const nk_addr_t* addr);

NETKIT_API char* nk_addr_get_ip(const nk_addr_t* addr);
NETKIT_API char* nk_addr_get_path(const nk_addr_t* addr);
NETKIT_API char* nk_addr_get_hostname(const nk_addr_t* addr);
NETKIT_API int nk_addr_get_port(const nk_addr_t* addr);
NETKIT_API nk_addr_type_t nk_addr_get_type(const nk_addr_t* addr);

NETKIT_API nk_status_t nk_socket_create(const nk_addr_t* addr, nk_socket_type_t type, uint32_t opts, nk_socket_t** out);
NETKIT_API void nk_socket_destroy(nk_socket_t* sock);
NETKIT_API nk_status_t nk_socket_connect(nk_socket_t* sock);

NETKIT_API nk_status_t nk_socket_send(nk_socket_t* sock, const void* buf, size_t len, size_t* out_sent);
NETKIT_API nk_status_t nk_socket_recv(nk_socket_t* sock, void* buf, size_t len, size_t* out_received);
NETKIT_API nk_status_t nk_socket_sendto(nk_socket_t* sock, const void* buf, size_t len, const nk_addr_t* dest, size_t* out_sent);
NETKIT_API nk_status_t nk_socket_recvfrom(nk_socket_t* sock, void* buf, size_t len, size_t* out_received, nk_addr_t** out_from);
NETKIT_API nk_status_t nk_socket_bind(nk_socket_t* sock);
NETKIT_API nk_status_t nk_socket_bind_addr(nk_socket_t* sock, const nk_addr_t* addr);
NETKIT_API void nk_socket_unbind(nk_socket_t* sock);
NETKIT_API bool nk_socket_is_open(const nk_socket_t* sock);
NETKIT_API void nk_socket_close(nk_socket_t* sock);
NETKIT_API intptr_t nk_socket_native_handle(const nk_socket_t* sock);
NETKIT_API nk_status_t nk_socket_set_opts(nk_socket_t* sock, uint32_t opts);
NETKIT_API nk_status_t nk_socket_get_peer(const nk_socket_t* sock, nk_addr_t** out);
NETKIT_API nk_status_t nk_socket_get_addr(nk_socket_t* sock, nk_addr_t** out);

NETKIT_API nk_status_t nk_listener_create(const nk_addr_t* addr, nk_socket_type_t type, uint32_t opts, nk_listener_t** out);
NETKIT_API void nk_listener_destroy(nk_listener_t* listener);
NETKIT_API nk_status_t nk_listener_bind(nk_listener_t* listener);
NETKIT_API nk_status_t nk_listener_bind_addr(nk_listener_t* listener, const nk_addr_t* addr);
NETKIT_API nk_status_t nk_listener_unbind(nk_listener_t* listener);
NETKIT_API nk_status_t nk_listener_listen(nk_listener_t* listener, int backlog);
NETKIT_API nk_status_t nk_listener_listen_default(nk_listener_t* listener);
NETKIT_API nk_status_t nk_listener_accept(nk_listener_t* listener, nk_socket_t** out);
NETKIT_API void nk_listener_close(nk_listener_t* listener);
NETKIT_API nk_status_t nk_listener_get_local_endpoint(const nk_listener_t* listener, nk_addr_t** out);
NETKIT_API intptr_t nk_listener_native_handle(const nk_listener_t* listener);

#ifdef __cplusplus
}
#endif
