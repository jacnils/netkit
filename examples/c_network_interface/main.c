#include <netkit/c/netkit.h>
#include <stdio.h>

int main(void) {
	nk_network_interface_t* interfaces = NULL;
	size_t count = 0;

	if (nk_network_get_interfaces(&interfaces, &count) != NK_OK) {
		fprintf(stderr, "failed: %s\n", nk_last_error());
		return 1;
	}

	for (size_t i = 0; i < count; ++i) {
		const nk_network_interface_t* iface = &interfaces[i];
		printf("%s (up=%s)\n", iface->name, iface->up ? "yes" : "no");

		for (size_t j = 0; j < iface->ipv4_count; ++j)
			printf("  ipv4: %s\n", iface->ipv4[j].ip);
		for (size_t j = 0; j < iface->ipv6_count; ++j)
			printf("  ipv6: %s\n", iface->ipv6[j].ip);
	}

	nk_network_interfaces_free(interfaces, count);
	return 0;
}