#ifndef NETKIT_SSL_SYNC_SOCK_H
#define NETKIT_SSL_SYNC_SOCK_H
#include "netkit-c/sock/sync_sock.h"
#ifdef __cplusplus
extern "C" {
#endif

#include <netkit-c/definitions.h>

#ifdef NETKIT_OPENSSL

#include <netkit-c/sock/sock_addr.h>
#include <netkit-c/sock/sock_types.h>
#include <netkit-c/sock/sync_sock.h>
#include <netkit-c/export.h>
#include <stddef.h>

typedef enum netkit_ssl_sync_sock_mode {
	NETKIT_SSL_SYNC_SOCK_MODE_CLIENT,
	NETKIT_SSL_SYNC_SOCK_MODE_SERVER,
} netkit_ssl_sync_sock_mode_t;

typedef enum netkit_ssl_sync_sock_version {
	NETKIT_SSL_SYNC_SOCK_VERSION_SSL_2,
	NETKIT_SSL_SYNC_SOCK_VERSION_SSL_3,
	NETKIT_SSL_SYNC_SOCK_VERSION_TLS_1_1,
	NETKIT_SSL_SYNC_SOCK_VERSION_TLS_1_2,
	NETKIT_SSL_SYNC_SOCK_VERSION_TLS_1_3,
} netkit_ssl_sync_sock_version_t;

typedef enum netkit_ssl_sync_sock_verification {
	NETKIT_SSL_SYNC_SOCK_VERIFICATION_NONE,
	NETKIT_SSL_SYNC_SOCK_VERIFICATION_PEER,
} netkit_ssl_sync_sock_verification_t;

typedef struct netkit_ssl_sync_sock netkit_ssl_sync_sock_t;

NETKIT_C_API netkit_ssl_sync_sock_t* netkit_ssl_sync_sock_create(netkit_sync_sock_t* sock,
			                                                 	 netkit_ssl_sync_sock_mode_t mode,
			                                                 	 netkit_ssl_sync_sock_version_t version,
			                                                 	 netkit_ssl_sync_sock_verification_t verification,
			                                                 	 const char* cert_path,
			                                                 	 const char* key_path);
NETKIT_C_API netkit_sock_status_t netkit_ssl_sync_sock_destroy(netkit_ssl_sync_sock_t* sock);
NETKIT_C_API netkit_sock_status_t netkit_ssl_sync_sock_connect(netkit_ssl_sync_sock_t* sock);
NETKIT_C_API netkit_sock_status_t netkit_ssl_sync_sock_bind(netkit_ssl_sync_sock_t* sock);
NETKIT_C_API netkit_sock_status_t netkit_ssl_sync_sock_unbind(netkit_ssl_sync_sock_t* sock);
NETKIT_C_API netkit_sock_status_t netkit_ssl_sync_sock_listen(netkit_ssl_sync_sock_t* sock);
NETKIT_C_API netkit_sock_status_t netkit_ssl_sync_sock_listen_n(netkit_ssl_sync_sock_t* sock, int backlog);
NETKIT_C_API netkit_sock_status_t netkit_ssl_sync_sock_perform_handshake(netkit_ssl_sync_sock_t* sock);
NETKIT_C_API netkit_ssl_sync_sock_t* netkit_ssl_sync_sock_accept(netkit_ssl_sync_sock_t* sock);
NETKIT_C_API int netkit_ssl_sync_sock_send(netkit_ssl_sync_sock_t* sock, void* buf, size_t len);
NETKIT_C_API void netkit_ssl_sync_sock_overflow_bytes(netkit_ssl_sync_sock_t* sock, char* buf, size_t len, size_t* out_len);
NETKIT_C_API void netkit_ssl_sync_sock_clear_overflow_bytes(netkit_ssl_sync_sock_t* sock);

NETKIT_C_API netkit_recv_status_t netkit_ssl_sync_sock_recv(netkit_ssl_sync_sock_t* sock, netkit_recv_result_t* out, int timeout_seconds, const char* match, size_t eof);
NETKIT_C_API void netkit_ssl_sync_sock_close(netkit_ssl_sync_sock_t* sock);
NETKIT_C_API netkit_sock_addr_t* netkit_ssl_sync_sock_get_peer(netkit_ssl_sync_sock_t* sock);

#endif

#ifdef __cplusplus
}
#endif
#endif
