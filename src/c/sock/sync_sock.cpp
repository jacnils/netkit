#include <cstddef>
#include <cstring>
#include <netkit-c/sock/sock_addr.h>
#include <netkit-c/sock/sock_addr_types.h>
#include <netkit-c/sock/sync_sock.h>
#include <netkit/sock/sync_sock.hpp>

struct netkit_sync_sock {
	std::unique_ptr<netkit::sock::basic_sync_sock> impl;
	explicit netkit_sync_sock() = default;
	static netkit::sock::type get_type(netkit_sock_type_t type) {
		switch (type) {
			case SOCK_UDP:
				return netkit::sock::type::udp;
			case SOCK_UNIX:
				return netkit::sock::type::unix;
			default:
				return netkit::sock::type::tcp;
		}
	}
	static netkit::sock::addr_type get_addr_type(netkit_sock_addr_type_t type) {
		switch (type) {
			case SOCK_ADDR_FILENAME:
				return netkit::sock::addr_type::filename;
			case SOCK_ADDR_IPV4:
				return netkit::sock::addr_type::ipv4;
			case SOCK_ADDR_IPV6:
				return netkit::sock::addr_type::ipv6;
			case SOCK_ADDR_HOSTNAME_IPV4:
				return netkit::sock::addr_type::hostname_ipv4;
			case SOCK_ADDR_HOSTNAME_IPV6:
				return netkit::sock::addr_type::hostname_ipv6;
			default:
				return netkit::sock::addr_type::hostname;
		}
	}
	static netkit_sock_addr_type_t get_addr_type(netkit::sock::addr_type type) {
		switch (type) {
		case netkit::sock::addr_type::filename:
			return SOCK_ADDR_FILENAME;
		case netkit::sock::addr_type::ipv4:
			return SOCK_ADDR_IPV4;
		case netkit::sock::addr_type::ipv6:
			return SOCK_ADDR_IPV6;
		case netkit::sock::addr_type::hostname_ipv4:
			return SOCK_ADDR_HOSTNAME_IPV4;
		case netkit::sock::addr_type::hostname_ipv6:
			return SOCK_ADDR_HOSTNAME_IPV6;
		default:
			return SOCK_ADDR_HOSTNAME;
		}
	}
	static netkit::sock::opt get_opt(netkit_sock_opt_t type) {
		netkit::sock::opt opt{};

		if (SOCK_OPT_HAS(type, SOCK_OPT_NONE)) {
			return netkit::sock::opt::no_reuse_addr | netkit::sock::opt::no_delay | netkit::sock::opt::blocking;
		}

		if (SOCK_OPT_HAS(type, SOCK_OPT_BLOCKING)) {
			opt = opt | netkit::sock::opt::blocking;
		} else if (SOCK_OPT_HAS(type, SOCK_OPT_NON_BLOCKING)) {
			opt = opt | netkit::sock::opt::no_blocking;
		}
		if (SOCK_OPT_HAS(type, SOCK_OPT_KEEP_ALIVE)) {
			opt = opt | netkit::sock::opt::keep_alive;
		} else if (SOCK_OPT_HAS(type, SOCK_OPT_NO_KEEP_ALIVE)) {
			opt = opt | netkit::sock::opt::no_keep_alive;
		}
		if (SOCK_OPT_HAS(type, SOCK_OPT_REUSE_ADDR)) {
			opt = opt | netkit::sock::opt::reuse_addr;
		} else if (SOCK_OPT_HAS(type, SOCK_OPT_NO_REUSE_ADDR)) {
			opt = opt | netkit::sock::opt::no_reuse_addr;
		}

		return opt;
	}
	static netkit::sock::addr get_sock_addr(netkit_sock_addr_t* addr) {
		if (!addr) {
			throw std::invalid_argument("invalid sock_addr");
		}

		size_t len = 0;
		netkit_sock_addr_get_hostname(addr, nullptr, 0, &len);

		if (len == 0) {
			throw std::runtime_error("hostname length is 0");
		}

		std::string hostname(len, '\0');

		netkit_sock_addr_get_hostname(addr, hostname.data(), len, nullptr);

		if (!hostname.empty() && hostname.back() == '\0') {
			hostname.pop_back();
		}

		return {
			hostname,
			netkit_sock_addr_get_port(addr),
			get_addr_type(netkit_sock_addr_get_type(addr))
		};
	}
	netkit_sync_sock(netkit_sock_addr_t* addr, netkit_sock_type_t type, netkit_sock_opt_t opts) {
		if (!addr) {
			throw std::invalid_argument("netkit_sock_addr_t* addr is null");
		}

		impl = std::make_unique<netkit::sock::sync_sock>(get_sock_addr(addr), get_type(type), get_opt(opts));
	}
};

netkit_sync_sock_t* from_cstyle(std::unique_ptr<netkit::sock::basic_sync_sock> ptr) {
	auto* sync_sock = new netkit_sync_sock();

	sync_sock->impl = std::move(ptr);

	return sync_sock;
}

extern "C" NETKIT_C_API netkit_sync_sock_t* netkit_sync_sock_create(netkit_sock_addr_t* addr, netkit_sock_type_t type, netkit_sock_opt_t opts) {
	if (!addr) {
		return nullptr;
	}

	try {
		return new netkit_sync_sock(addr, type, opts);
	} catch (...) {
		return nullptr;
	}
}

extern "C" NETKIT_C_API netkit_sock_status_t netkit_sync_sock_destroy(netkit_sync_sock_t* sock) {
	if (!sock) {
		return SOCK_STATUS_FAILED;
	}

	try {
		delete sock;
		return SOCK_STATUS_SUCCESS;
	} catch (...) {
	}

	return SOCK_STATUS_FAILED;
}

extern "C" NETKIT_C_API netkit_sock_status_t netkit_sync_sock_connect(netkit_sync_sock_t* sock) {
	if (!sock) {
		return SOCK_STATUS_FAILED;
	}

	try {
		sock->impl->connect();
	} catch (...) {
		return SOCK_STATUS_FAILED;
	}

	return SOCK_STATUS_SUCCESS;
}

extern "C" NETKIT_C_API netkit_sock_status_t netkit_sync_sock_bind(netkit_sync_sock_t* sock) {
	if (!sock) {
		return SOCK_STATUS_FAILED;
	}

	try {
		sock->impl->bind();
	} catch (...) {
		return SOCK_STATUS_FAILED;
	}

	return SOCK_STATUS_SUCCESS;
}
extern "C" NETKIT_C_API netkit_sock_status_t netkit_sync_sock_unbind(netkit_sync_sock_t* sock) {
	if (!sock) {
		return SOCK_STATUS_FAILED;
	}

	try {
		sock->impl->unbind();
	} catch (...) {
		return SOCK_STATUS_FAILED;
	}

	return SOCK_STATUS_SUCCESS;
}
extern "C" NETKIT_C_API netkit_sock_status_t netkit_sync_sock_listen(netkit_sync_sock_t* sock) {
	if (!sock) {
		return SOCK_STATUS_FAILED;
	}

	try {
		sock->impl->listen();
	} catch (...) {
		return SOCK_STATUS_FAILED;
	}

	return SOCK_STATUS_SUCCESS;
}

extern "C" NETKIT_C_API netkit_sock_status_t netkit_sync_sock_listen_n(netkit_sync_sock_t* sock, int backlog) {
	if (!sock) {
		return SOCK_STATUS_FAILED;
	}

	try {
		sock->impl->listen(backlog);
	} catch (...) {
		return SOCK_STATUS_FAILED;
	}

	return SOCK_STATUS_SUCCESS;
}
extern "C" NETKIT_C_API netkit_sync_sock_t* netkit_sync_sock_accept(netkit_sync_sock_t* sock) {
	if (!sock) {
		return nullptr;
	}

	try {
		return from_cstyle(sock->impl->accept());
	} catch (...) {
		return nullptr;
	}
}
extern "C" NETKIT_C_API int netkit_sync_sock_send(netkit_sync_sock_t* sock, void* buf, size_t len) {
	if (!sock) {
		return -1;
	}

	try {
		return sock->impl->send(buf, len);
	} catch (...) {
		return -1;
	}
}
extern "C" NETKIT_C_API void netkit_sync_sock_overflow_bytes(netkit_sync_sock_t* sock, char* buf, size_t len, size_t* out_len) {
	if (!sock) {
		return;
	}

	try {
		const auto& bytes = sock->impl->overflow_bytes();

		if (out_len) {
			*out_len = bytes.size() + 1;
		}

		std::snprintf(buf, len, "%s", bytes.c_str());
	} catch (...) {}
}
extern "C" NETKIT_C_API void netkit_sync_sock_clear_overflow_bytes(netkit_sync_sock_t* sock) {
	if (!sock) {
		return;
	}

	try {
		sock->impl->clear_overflow_bytes();
	} catch (...) {}
}

extern "C" NETKIT_C_API netkit_recv_result_t* netkit_recv_result_create(void) {
	return new netkit_recv_result_t{nullptr, 0};
}
extern "C" NETKIT_C_API void netkit_recv_result_destroy(netkit_recv_result_t* recv_result) {
	if (!recv_result) {
		return;
	}
	delete[] recv_result->data;
	recv_result->data = nullptr;
	delete recv_result;
}

extern "C" NETKIT_C_API netkit_recv_status_t netkit_sync_sock_recv(netkit_sync_sock_t* sock, netkit_recv_result_t* out, int timeout_seconds, const char* match, size_t eof) {
	if (!sock || !out) {
		return RECV_ERROR;
	}

	try {
		auto ret = sock->impl->recv(timeout_seconds, match ? match : "", eof);

		delete[] out->data;

		out->size = ret.data.size();
		out->data = new char[out->size + 1];

		std::memcpy(out->data, ret.data.data(), out->size);

		out->data[out->size] = '\0';

		switch (ret.status) {
			case netkit::sock::recv_status::success:
				return RECV_SUCCESS;
			case netkit::sock::recv_status::timeout:
				return RECV_TIMEOUT;
			case netkit::sock::recv_status::closed:
				return RECV_CLOSED;
			default:
				return RECV_ERROR;
		}
	} catch (const std::exception& e) {
		return RECV_ERROR;
	} catch (...) {
		return RECV_ERROR;
	}
}

extern "C" NETKIT_C_API void netkit_sync_sock_close(netkit_sync_sock_t* sock) {
	if (!sock) {
		return;
	}

	try {
		sock->impl->close();
	} catch (...) {}
}
extern "C" NETKIT_C_API netkit_sock_addr_t* netkit_sync_sock_get_peer(netkit_sync_sock_t* sock) {
	if (!sock) {
		return nullptr;
	}

	try {
		auto impl_peer = sock->impl->get_peer();
		return netkit_sock_addr_create(impl_peer.get_hostname().c_str(), impl_peer.get_port(), netkit_sync_sock::get_addr_type(impl_peer.get_type()));
	} catch (...) {
		return nullptr;
	}
}