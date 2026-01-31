/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file main.c
 *  @license MIT
 *  @note Example code using the Netkit library.
 *  @brief A lower-level example demonstrating the usage of sync_sock to make a simple HTTP request.
 */
#include <assert.h>
#include <netkit-c/sock/sock_addr.h>
#include <netkit-c/sock/sync_sock.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
	netkit_sock_addr_t* addr = netkit_sock_addr_create("google.com", 80, NETKIT_SOCK_ADDR_HOSTNAME);
	if (!addr) {
		return 1;
	}

	netkit_sync_sock_t* sock = netkit_sync_sock_create(addr, NETKIT_SOCK_TCP, NETKIT_SOCK_OPT_NONE);
	if (!sock) {
		fprintf(stderr, "%s\n", "Failed to create socket.");
		return 1;
	}

	if (netkit_sync_sock_connect(sock) != NETKIT_SOCK_STATUS_SUCCESS) {
		fprintf(stderr, "%s\n", "Failed to connect.");
	}

	char* buf = "GET / HTTP/1.1\r\nHost: google.com\r\nConnection: close\r\n\r\n";

	if (!netkit_sync_sock_send(sock, buf, strlen(buf))) {
		fprintf(stderr, "%s\n", "Failed to send.");
	}

	netkit_recv_result_t* result = netkit_recv_result_create();
	if (!result) {
		fprintf(stderr, "%s\n", "Failed to create recv_result");
		return 1;
	}

	netkit_recv_status_t status = netkit_sync_sock_recv(sock, result, 5, "", 0);
	if (status == NETKIT_RECV_ERROR) {
		fprintf(stderr, "Error.\n");
		return 1;
	} else if (status == NETKIT_RECV_TIMEOUT) {
		fprintf(stderr, "Timeout.\n");
		return 1;
	}
	netkit_sync_sock_close(sock);

	if (result->data) {
		fprintf(stdout, "%s\n", result->data);
	} else {
		fprintf(stderr, "%s\n", "result->data == NULL");
		return 1;
	}

	netkit_recv_result_destroy(result);
	netkit_sock_addr_destroy(addr);
	netkit_sync_sock_destroy(sock);
}