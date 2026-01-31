#include <cstddef>

#include <netkit-c/sock/sock_addr.h>
#include <netkit-c/sock/sock_addr_types.h>

#include <netkit/sock/sock_addr.hpp>

struct netkit_sock_addr {
	std::unique_ptr<netkit::sock::sock_addr> impl;
	netkit_sock_addr_type_t impl_type = NETKIT_SOCK_ADDR_NONE;

	explicit netkit_sock_addr() = default;
	explicit netkit_sock_addr(const char* file_path) : impl_type(NETKIT_SOCK_ADDR_FILENAME) {
		impl = std::make_unique<netkit::sock::sock_addr>(file_path);
	}

	netkit_sock_addr(const char* hostname, int port, netkit_sock_addr_type_t type) : impl_type(type) {
		netkit::sock::sock_addr_type t;

		switch (type) {
			case NETKIT_SOCK_ADDR_FILENAME:
				t = netkit::sock::sock_addr_type::filename;
				break;
			case NETKIT_SOCK_ADDR_HOSTNAME:
				t = netkit::sock::sock_addr_type::hostname;
				break;
			case NETKIT_SOCK_ADDR_HOSTNAME_IPV4:
				t = netkit::sock::sock_addr_type::hostname_ipv4;
				break;
			case NETKIT_SOCK_ADDR_HOSTNAME_IPV6:
				t = netkit::sock::sock_addr_type::hostname_ipv6;
				break;
			case NETKIT_SOCK_ADDR_IPV6:
				t = netkit::sock::sock_addr_type::ipv6;
				break;
			case NETKIT_SOCK_ADDR_IPV4:
				t = netkit::sock::sock_addr_type::ipv4;
				break;
			default:
				t = netkit::sock::sock_addr_type::hostname;
				break;
		}

		impl = std::make_unique<netkit::sock::sock_addr>(hostname, port, t);
	}
};

extern "C" NETKIT_C_API netkit_sock_addr_t* netkit_sock_addr_create_unix(const char* file_path) {
	netkit_sock_addr_t* ret;

	try {
		ret = new netkit_sock_addr{file_path};
	} catch (std::exception&) {
		return nullptr;
	} catch (...) {
		return nullptr;
	}

	return ret;
}

extern "C" NETKIT_C_API netkit_sock_addr_t* netkit_sock_addr_create(const char* hostname, int port, netkit_sock_addr_type_t type) {
	netkit_sock_addr_t* ret;

	try {
		ret = new netkit_sock_addr{hostname, port, type};
	} catch (std::exception&) {
		return nullptr;
	} catch (...) {
		return nullptr;
	}

	return ret;
}

extern "C" NETKIT_C_API void netkit_sock_addr_destroy(netkit_sock_addr_t* addr) {
	if (!addr) {
		return;
	}

	delete addr;
}

extern "C" NETKIT_C_API int netkit_sock_addr_get_port(netkit_sock_addr_t* addr) {
	if (!addr) {
		return -1;
	}

	try {
		return addr->impl->get_port();
	} catch (std::exception&) {
		return -1;
	} catch (...) {
		return -1;
	}
}

extern "C" NETKIT_C_API netkit_sock_addr_type_t netkit_sock_addr_get_type(netkit_sock_addr_t* addr) {
	if (!addr) {
		return NETKIT_SOCK_ADDR_NONE;
	}

	return addr->impl_type;
}

extern "C" NETKIT_C_API netkit_sock_addr_status_t netkit_sock_addr_get_hostname(netkit_sock_addr_t* addr, char* hostname, size_t len, size_t* out_len) {
	if (!addr) {
		return NETKIT_SOCK_ADDR_STATUS_FAILED;
	}

	try {
		const auto& host = addr->impl->get_hostname();

		if (out_len) {
			*out_len = host.size() + 1;
		}

		if (hostname) {
			if (len == 0) {
				return NETKIT_SOCK_ADDR_STATUS_FAILED;
			}

			std::snprintf(hostname, len, "%s", host.c_str());
		}

		return NETKIT_SOCK_ADDR_STATUS_SUCCESS;
	} catch (...) {
		return NETKIT_SOCK_ADDR_STATUS_FAILED;
	}
}