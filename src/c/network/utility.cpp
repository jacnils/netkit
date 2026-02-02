#include <cstddef>
#include <netkit-c/export.h>
#include <netkit-c/network/utility.h>
#include <netkit/network/utility.hpp>

extern "C" NETKIT_C_API int netkit_network_is_ipv4(const char* ip, size_t len) {
	if (!ip || len == 0) {
		return 0;
	}

	try {
		std::string ip_str{ip, len};
		return netkit::network::is_ipv4(ip_str) ? 1 : 0;
	} catch (std::exception&) {
		return 0;
	} catch (...) {
		return 0;
	}
}

extern "C" NETKIT_C_API int netkit_network_is_ipv6(const char* ip, size_t len) {
	if (!ip || len == 0) {
		return 0;
	}

	try {
		std::string ip_str{ip, len};
		return netkit::network::is_ipv6(ip_str) ? 1 : 0;
	} catch (std::exception&) {
		return 0;
	} catch (...) {
		return 0;
	}
}

extern "C" NETKIT_C_API int netkit_network_is_valid_port(int port) {
	try {
		return netkit::network::is_valid_port(port) ? 1 : 0;
	} catch (std::exception&) {
		return 0;
	} catch (...) {
		return 0;
	}
}

extern "C" NETKIT_C_API int netkit_network_usable_ipv6_address_exists() {
	try {
		return netkit::network::usable_ipv6_address_exists() ? 1 : 0;
	} catch (std::exception&) {
		return 0;
	} catch (...) {
		return 0;
	}
}