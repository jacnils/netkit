#ifndef NETKIT_SOCK_ADDR_TYPES_H
#define NETKIT_SOCK_ADDR_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum netkit_sock_addr_status {
	NETKIT_SOCK_ADDR_STATUS_SUCCESS,
	NETKIT_SOCK_ADDR_STATUS_FAILED,
} netkit_sock_addr_status_t;

typedef enum netkit_sock_addr_type {
	NETKIT_SOCK_ADDR_NONE = 0,
	NETKIT_SOCK_ADDR_IPV4 = 1,
	NETKIT_SOCK_ADDR_IPV6 = 2,
	NETKIT_SOCK_ADDR_HOSTNAME_IPV4 = 3,
	NETKIT_SOCK_ADDR_HOSTNAME_IPV6 = 4,
	NETKIT_SOCK_ADDR_HOSTNAME = 5,
	NETKIT_SOCK_ADDR_FILENAME = 6
} netkit_sock_addr_type_t;

#ifdef __cplusplus
}
#endif
#endif
