#include <assert.h>
#include <netkit-c/sock/sock_addr.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
	netkit_sock_addr_t* addr = netkit_sock_addr_create("google.com", 80, NETKIT_SOCK_ADDR_HOSTNAME);

	assert(addr != NULL);

	size_t len;
	netkit_sock_addr_get_hostname(addr, NULL, 0, &len);

	char* hostname = malloc(len);
	netkit_sock_addr_get_hostname(addr, hostname, len, &len);

	assert(!strncmp(hostname, "google.com", len));
	assert(80 == netkit_sock_addr_get_port(addr));
	assert(NETKIT_SOCK_ADDR_HOSTNAME == netkit_sock_addr_get_type(addr));

	free(hostname);

	netkit_sock_addr_destroy(addr);
}