#ifndef NETKIT_SOCK_TYPES_H
#define NETKIT_SOCK_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <netkit-c/export.h>
#include <stdint.h>

#ifndef SOCK_OPT_HAS
#define SOCK_OPT_HAS(mask, opt) (((mask) & (opt)) != 0)
#endif
#ifndef SOCK_OPT

typedef enum netkit_sock_type {
	SOCK_NONE,
	SOCK_TCP,
	SOCK_UDP,
	SOCK_UNIX
} netkit_sock_type_t;

typedef enum netkit_sock_status {
	SOCK_STATUS_FAILED = 1,
	SOCK_STATUS_SUCCESS = 0,
} netkit_sock_status_t;

typedef enum netkit_sock_opt {
	SOCK_OPT_NONE           = 1 << 0,
	SOCK_OPT_REUSE_ADDR     = 1 << 1,
	SOCK_OPT_NO_REUSE_ADDR  = 1 << 2,
	SOCK_OPT_NO_DELAY       = 1 << 3,
	SOCK_OPT_KEEP_ALIVE     = 1 << 4,
	SOCK_OPT_NO_KEEP_ALIVE  = 1 << 5,
	SOCK_OPT_NON_BLOCKING   = 1 << 6,
	SOCK_OPT_BLOCKING       = 1 << 7
} netkit_sock_opt_t;

typedef enum netkit_recv_status {
	RECV_SUCCESS,
	RECV_TIMEOUT,
	RECV_CLOSED,
	RECV_ERROR,
} netkit_recv_status_t;

typedef struct netkit_recv_result {
	char* data;
	size_t size;
} netkit_recv_result_t;

#ifdef __cplusplus
}
#endif
#endif
#endif