#include <netkit-c/network/utility.h>
#include <assert.h>
#include <string.h>
#include "definitions.h"

void test_network_utility(void) {
	char* ipv4 = "0.1.0.1";
	ASSERT(netkit_network_is_ipv4(ipv4, strlen(ipv4)) == 1);

	char* ipv6 = "2001:0db8:85a3:0000:0000:8a2e:0370:7334";
	ASSERT(netkit_network_is_ipv6(ipv6, strlen(ipv6)) == 1);

	char* invalid_ip = "999.999.999.999";
	ASSERT(netkit_network_is_ipv4(invalid_ip, strlen(invalid_ip)) == 0);

	char* invalid_ipv6 = "2001:0db8:85a3:0000:0000:8a2e:0370:zzzz";
	ASSERT(netkit_network_is_ipv6(invalid_ipv6, strlen(invalid_ipv6)) == 0);

	ASSERT(netkit_network_is_valid_port(8080) == 1);
	ASSERT(netkit_network_is_valid_port(70000) == 0);
	ASSERT(netkit_network_usable_ipv6_address_exists() == 1 || netkit_network_usable_ipv6_address_exists() == 0);
}