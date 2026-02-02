/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file main.c
 *  @license MIT
 *  @note Example code using the Netkit library.
 *  @brief A lower-level example demonstrating the usage of ssl_sync_sock to make a simple HTTP request with TLS/SSL support.
 */
#include <assert.h>
#include <netkit-c/sock/sock_addr.h>
#include <netkit-c/sock/sync_sock.h>
#include <netkit-c/sock/openssl/ssl_sync_sock.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TRIES 3

int main(void) {
	netkit_sock_addr_t* addr = netkit_sock_addr_create("www.google.com", 443, SOCK_ADDR_HOSTNAME);
	if (!addr) {
		return 1;
	}

	netkit_sync_sock_t* sock = netkit_sync_sock_create(addr, SOCK_TCP, SOCK_OPT_NONE);
	if (!sock) {
		fprintf(stderr, "%s\n", "Failed to create socket.");
		return 1;
	}

	if (netkit_sync_sock_connect(sock) != SOCK_STATUS_SUCCESS) {
		fprintf(stderr, "%s\n", "Failed to connect.");
		return 1;
	}

	netkit_ssl_sync_sock_t* ssl_sock = netkit_ssl_sync_sock_create(
		sock,
		NETKIT_SSL_SYNC_SOCK_MODE_CLIENT,
		NETKIT_SSL_SYNC_SOCK_VERSION_TLS_1_2,
		NETKIT_SSL_SYNC_SOCK_VERIFICATION_PEER,
		NULL,
		NULL
		);
	if (!ssl_sock) {
		fprintf(stderr, "%s\n", "Failed to create SSL sync sock.");
		return 1;
	}

	if (netkit_ssl_sync_sock_perform_handshake(ssl_sock)) {
		fprintf(stderr, "%s\n", "Failed to perform handshake.");
		return 1;
	}

	char* buf = "GET / HTTP/1.1\r\nHost: www.google.com\r\nConnection: close\r\n\r\n";

	if (!netkit_ssl_sync_sock_send(ssl_sock, buf, strlen(buf))) {
		fprintf(stderr, "%s\n", "Failed to send.");
		return 1;
	}

	netkit_recv_result_t* result = netkit_recv_result_create();
	if (!result) {
		fprintf(stderr, "%s\n", "Failed to create recv_result");
		return 1;
	}

	int tries = 0;

	while (tries++ < MAX_TRIES) {
		netkit_recv_status_t status = netkit_ssl_sync_sock_recv(ssl_sock, result, -1, NULL, 0);
		if (status == RECV_TIMEOUT) {
			fprintf(stderr, "%s\n", "Failed to receive: timeout.");
			return 1;
		} else if (status == RECV_ERROR) {
			fprintf(stderr, "%s\n", "Failed to receive: error.");
			return 1;
		}
	}

	netkit_ssl_sync_sock_close(ssl_sock);

	if (result->data) {
		fprintf(stdout, "%s\n", result->data);
	} else {
		fprintf(stderr, "%s\n", "result->data == NULL");
		return 1;
	}

	netkit_recv_result_destroy(result);
	netkit_sock_addr_destroy(addr);
	netkit_ssl_sync_sock_destroy(ssl_sock);
	netkit_sync_sock_destroy(sock);

	return 0;
}
