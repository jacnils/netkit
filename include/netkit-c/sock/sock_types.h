#ifndef NETKIT_SOCK_TYPES_H
#define NETKIT_SOCK_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <netkit-c/export.h>
#include <stdint.h>

#ifndef NETKIT_SOCK_OPT_HAS
#define NETKIT_SOCK_OPT_HAS(mask, opt) (((mask) & (opt)) != 0)
#endif

typedef enum netkit_sock_type {
	NETKIT_SOCK_NONE,
	NETKIT_SOCK_TCP,
	NETKIT_SOCK_UDP,
	NETKIT_SOCK_UNIX
} netkit_sock_type_t;

typedef enum netkit_sock_status {
	NETKIT_SOCK_STATUS_FAILED = 1,
	NETKIT_SOCK_STATUS_SUCCESS = 0,
} netkit_sock_status_t;

typedef enum netkit_sock_opt {
	NETKIT_SOCK_OPT_NONE           = 1 << 0,
	NETKIT_SOCK_OPT_REUSE_ADDR     = 1 << 1,
	NETKIT_SOCK_OPT_NO_REUSE_ADDR  = 1 << 2,
	NETKIT_SOCK_OPT_NO_DELAY       = 1 << 3,
	NETKIT_SOCK_OPT_KEEP_ALIVE     = 1 << 4,
	NETKIT_SOCK_OPT_NO_KEEP_ALIVE  = 1 << 5,
	NETKIT_SOCK_OPT_NON_BLOCKING   = 1 << 6,
	NETKIT_SOCK_OPT_BLOCKING       = 1 << 7
} netkit_sock_opt_t;

typedef enum netkit_recv_status {
	NETKIT_RECV_SUCCESS,
	NETKIT_RECV_TIMEOUT,
	NETKIT_RECV_CLOSED,
	NETKIT_RECV_ERROR,
} netkit_recv_status_t;

typedef struct netkit_recv_result {
	char* data;
	size_t size;
} netkit_recv_result_t;

#ifdef __cplusplus
}
#endif
#endif
