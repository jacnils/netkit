#ifndef NETKIT_SOCK_ADDR_H
#define NETKIT_SOCK_ADDR_H
#ifdef __cplusplus
extern "C" {
#endif

#include <netkit-c/sock/sock_addr_types.h>
#include <netkit-c/export.h>
#include <stddef.h>

typedef struct netkit_sock_addr netkit_sock_addr_t;

NETKIT_C_API netkit_sock_addr_t* netkit_sock_addr_create(const char* hostname, int port, netkit_sock_addr_type_t type);
NETKIT_C_API netkit_sock_addr_t* netkit_sock_addr_create_unix(const char* file_path);
NETKIT_C_API void netkit_sock_addr_destroy(netkit_sock_addr_t* addr);
NETKIT_C_API int netkit_sock_addr_get_port(netkit_sock_addr_t* addr);
NETKIT_C_API netkit_sock_addr_type_t netkit_sock_addr_get_type(netkit_sock_addr_t* addr);
NETKIT_C_API netkit_sock_addr_status_t netkit_sock_addr_get_hostname(netkit_sock_addr_t* addr, char* hostname, size_t len, size_t* out_len);

#ifdef __cplusplus
}
#endif
#endif
