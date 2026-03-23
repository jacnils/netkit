#ifndef NETKIT_SYNC_SOCK_H
#define NETKIT_SYNC_SOCK_H
#ifdef __cplusplus
extern "C" {
#endif

#include <netkit-c/sock/sock_addr.h>
#include <netkit-c/sock/sock_types.h>
#include <netkit-c/export.h>
#include <stddef.h>

typedef struct netkit_sync_sock netkit_sync_sock_t;

NETKIT_C_API netkit_sync_sock_t* netkit_sync_sock_create(netkit_sock_addr_t* addr, netkit_sock_type_t type, netkit_sock_opt_t opts);
NETKIT_C_API netkit_sock_status_t netkit_sync_sock_destroy(netkit_sync_sock_t* sock);
NETKIT_C_API netkit_sock_status_t netkit_sync_sock_connect(netkit_sync_sock_t* sock);
NETKIT_C_API netkit_sock_status_t netkit_sync_sock_bind(netkit_sync_sock_t* sock);
NETKIT_C_API netkit_sock_status_t netkit_sync_sock_unbind(netkit_sync_sock_t* sock);
NETKIT_C_API netkit_sock_status_t netkit_sync_sock_listen(netkit_sync_sock_t* sock);
NETKIT_C_API netkit_sock_status_t netkit_sync_sock_listen_n(netkit_sync_sock_t* sock, int backlog);
NETKIT_C_API netkit_sync_sock_t* netkit_sync_sock_accept(netkit_sync_sock_t* sock);
NETKIT_C_API int netkit_sync_sock_send(netkit_sync_sock_t* sock, void* buf, size_t len);
NETKIT_C_API void netkit_sync_sock_overflow_bytes(netkit_sync_sock_t* sock, char* buf, size_t len, size_t* out_len);
NETKIT_C_API void netkit_sync_sock_clear_overflow_bytes(netkit_sync_sock_t* sock);

NETKIT_C_API netkit_recv_result_t* netkit_recv_result_create(void);
NETKIT_C_API void netkit_recv_result_destroy(netkit_recv_result_t* recv_result);

NETKIT_C_API netkit_recv_status_t netkit_sync_sock_recv(netkit_sync_sock_t* sock, netkit_recv_result_t* out, int timeout_seconds, const char* match, size_t eof);
NETKIT_C_API netkit_recv_status_t netkit_sync_sock_basic_recv(netkit_sync_sock_t* sock, netkit_recv_result_t* out);
NETKIT_C_API void netkit_sync_sock_close(netkit_sync_sock_t* sock);
NETKIT_C_API netkit_sock_addr_t* netkit_sync_sock_get_peer(netkit_sync_sock_t* sock);

#ifdef __cplusplus
}
#endif
#endif
