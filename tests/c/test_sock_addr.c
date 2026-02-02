#include "definitions.h"

#include <assert.h>
#include <netkit-c/sock/sock_addr.h>
#include <netkit-c/sock/sync_sock.h>
#include <stdlib.h>
#include <string.h>

void test_sock_addr(void) {
	netkit_sock_addr_t* addr = netkit_sock_addr_create("google.com", 80, SOCK_ADDR_HOSTNAME);

	ASSERT(addr != NULL);

	size_t len;
	netkit_sock_addr_get_hostname(addr, NULL, 0, &len);

	char* hostname = malloc(len);
	netkit_sock_addr_get_hostname(addr, hostname, len, &len);

	ASSERT(!strncmp(hostname, "google.com", len));
	ASSERT(80 == netkit_sock_addr_get_port(addr));
	ASSERT(SOCK_ADDR_HOSTNAME == netkit_sock_addr_get_type(addr));

	free(hostname);
	netkit_sock_addr_destroy(addr);

	addr = netkit_sock_addr_create_unix("/tmp/socket");

	ASSERT(addr != NULL);
	ASSERT(netkit_sock_addr_get_type(addr) == SOCK_ADDR_FILENAME);

	netkit_sock_addr_destroy(addr);
}