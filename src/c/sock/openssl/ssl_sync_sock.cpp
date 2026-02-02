#ifdef NETKIT_OPENSSL

#include <iostream>
#include <cstring>
#include <netkit-c/sock/sock_addr.h>
#include <netkit-c/sock/sock_addr_types.h>
#include <netkit-c/sock/openssl/ssl_sync_sock.h>
#include <netkit/sock/openssl/ssl_sync_sock.hpp>

struct netkit_sync_sock {
	std::unique_ptr<netkit::sock::basic_sync_sock> impl;
};

struct netkit_ssl_sync_sock {
	std::unique_ptr<netkit::sock::ssl_sync_sock> impl;
	netkit_ssl_sync_sock() = default;

	static netkit::sock::mode to_mode(const netkit_ssl_sync_sock_mode_t mode) {
		switch (mode) {
			case NETKIT_SSL_SYNC_SOCK_MODE_CLIENT: return netkit::sock::mode::client;
			case NETKIT_SSL_SYNC_SOCK_MODE_SERVER: return netkit::sock::mode::server;
		}

		throw std::invalid_argument("invalid mode");
	}

	static netkit::sock::version to_version(const netkit_ssl_sync_sock_version_t version) {
		switch (version) {
			case NETKIT_SSL_SYNC_SOCK_VERSION_SSL_2: return netkit::sock::version::SSL_2;
			case NETKIT_SSL_SYNC_SOCK_VERSION_SSL_3: return netkit::sock::version::SSL_3;
			case NETKIT_SSL_SYNC_SOCK_VERSION_TLS_1_1: return netkit::sock::version::TLS_1_1;
			case NETKIT_SSL_SYNC_SOCK_VERSION_TLS_1_2: return netkit::sock::version::TLS_1_2;
			case NETKIT_SSL_SYNC_SOCK_VERSION_TLS_1_3: return netkit::sock::version::TLS_1_3;
		}

		throw std::invalid_argument("invalid version");
	}

	static netkit::sock::verification to_verification(const netkit_ssl_sync_sock_verification_t verification) {
		switch (verification) {
			case NETKIT_SSL_SYNC_SOCK_VERIFICATION_NONE: return netkit::sock::verification::none;
			case NETKIT_SSL_SYNC_SOCK_VERIFICATION_PEER: return netkit::sock::verification::peer;
		}

		throw std::invalid_argument("invalid verification");
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

	netkit_ssl_sync_sock(netkit_sync_sock_t* sock,
		netkit_ssl_sync_sock_mode_t mode,
		netkit_ssl_sync_sock_version_t version,
		netkit_ssl_sync_sock_verification_t verification,
		const char* cert_path,
		const char* key_path) {

		if (!sock->impl) {
			throw std::invalid_argument("invalid sock");
		}

		this->impl = std::make_unique<netkit::sock::ssl_sync_sock>(
			std::move(sock->impl),
			to_mode(mode),
			to_version(version),
			to_verification(verification),
			cert_path == nullptr ? "" : cert_path,
			key_path == nullptr ? "" : key_path
			);
	}
};

netkit_ssl_sync_sock_t* from_cstyle(std::unique_ptr<netkit::sock::ssl_sync_sock> ptr) {
	auto* sync_sock = new netkit_ssl_sync_sock();

	sync_sock->impl = std::move(ptr);

	return sync_sock;
}

extern "C" NETKIT_C_API netkit_ssl_sync_sock_t* netkit_ssl_sync_sock_create(netkit_sync_sock_t* sock,
		netkit_ssl_sync_sock_mode_t mode,
		netkit_ssl_sync_sock_version_t version,
		netkit_ssl_sync_sock_verification_t verification,
		const char* cert_path,
		const char* key_path) {

	if (!sock) {
		return nullptr;
	}

	try {
		return new netkit_ssl_sync_sock(sock, mode, version, verification, cert_path, key_path);
	} catch (...) {
		return nullptr;
	}
}

extern "C" NETKIT_C_API netkit_sock_status_t netkit_ssl_sync_sock_destroy(netkit_ssl_sync_sock_t* sock) {
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

extern "C" NETKIT_C_API netkit_sock_status_t netkit_ssl_sync_sock_connect(netkit_ssl_sync_sock_t* sock) {
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

extern "C" NETKIT_C_API netkit_sock_status_t netkit_ssl_sync_sock_bind(netkit_ssl_sync_sock_t* sock) {
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
extern "C" NETKIT_C_API netkit_sock_status_t netkit_ssl_sync_sock_unbind(netkit_ssl_sync_sock_t* sock) {
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
extern "C" NETKIT_C_API netkit_sock_status_t netkit_ssl_sync_sock_listen(netkit_ssl_sync_sock_t* sock) {
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

extern "C" NETKIT_C_API netkit_sock_status_t netkit_ssl_sync_sock_listen_n(netkit_ssl_sync_sock_t* sock, int backlog) {
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
extern "C" NETKIT_C_API netkit_ssl_sync_sock_t* netkit_ssl_sync_sock_accept(netkit_ssl_sync_sock_t* sock) {
	if (!sock) {
		return nullptr;
	}

	try {
		return from_cstyle(sock->impl->accept());
	} catch (...) {
		return nullptr;
	}
}
extern "C" NETKIT_C_API int netkit_ssl_sync_sock_send(netkit_ssl_sync_sock_t* sock, void* buf, size_t len) {
	if (!sock) {
		return -1;
	}

	try {
		return sock->impl->send(buf, len);
	} catch (...) {
		return -1;
	}
}
extern "C" NETKIT_C_API void netkit_ssl_sync_sock_overflow_bytes(netkit_ssl_sync_sock_t* sock, char* buf, size_t len, size_t* out_len) {
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
extern "C" NETKIT_C_API void netkit_ssl_sync_sock_clear_overflow_bytes(netkit_ssl_sync_sock_t* sock) {
	if (!sock) {
		return;
	}

	try {
		sock->impl->clear_overflow_bytes();
	} catch (...) {}
}

extern "C" NETKIT_C_API netkit_recv_status_t netkit_ssl_sync_sock_recv(netkit_ssl_sync_sock_t* sock, netkit_recv_result_t* out, int timeout_seconds, const char* match, size_t eof) {
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
	} catch (const std::exception&) {
		return RECV_ERROR;
	} catch (...) {
		return RECV_ERROR;
	}
}

extern "C" NETKIT_C_API void netkit_ssl_sync_sock_close(netkit_ssl_sync_sock_t* sock) {
	if (!sock) {
		return;
	}

	try {
		sock->impl->close();
	} catch (...) {}
}
extern "C" NETKIT_C_API netkit_sock_addr_t* netkit_ssl_sync_sock_get_peer(netkit_ssl_sync_sock_t* sock) {
	if (!sock) {
		return nullptr;
	}

	try {
		auto impl_peer = sock->impl->get_peer();
		return netkit_sock_addr_create(impl_peer.get_hostname().c_str(), impl_peer.get_port(), netkit_ssl_sync_sock::get_addr_type(impl_peer.get_type()));
	} catch (...) {
		return nullptr;
	}
}

extern "C" NETKIT_C_API netkit_sock_status_t netkit_ssl_sync_sock_perform_handshake(netkit_ssl_sync_sock_t* sock) {
	if (!sock) {
		return SOCK_STATUS_FAILED;
	}

	try {
		sock->impl->perform_handshake();
		return SOCK_STATUS_SUCCESS;
	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		return SOCK_STATUS_FAILED;
	} catch (...) {
		return SOCK_STATUS_FAILED;
	}
}
#endif