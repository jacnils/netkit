#include "internal.hpp"

#include <netkit/c/socket.h>

#include <netkit/socket/addr.hpp>
#include <netkit/socket/native/native_sync_listener.hpp>
#include <netkit/socket/native/native_sync_socket.hpp>

#include <memory>

using netkit::socket::addr;
using netkit::socket::native::basic_native_sync_socket;
using netkit::socket::native::native_sync_listener;
using netkit::socket::native::native_sync_socket;

namespace {
	addr& as_addr(nk_addr_t* a) { return *reinterpret_cast<addr*>(a); }
	const addr& as_addr(const nk_addr_t* a) { return *reinterpret_cast<const addr*>(a); }
	nk_addr_t* to_c(addr* a) { return reinterpret_cast<nk_addr_t*>(a); }

	basic_native_sync_socket& as_socket(nk_socket_t* s) { return *reinterpret_cast<basic_native_sync_socket*>(s); }
	const basic_native_sync_socket& as_socket(const nk_socket_t* s) {
		return *reinterpret_cast<const basic_native_sync_socket*>(s);
	}
	nk_socket_t* to_c(basic_native_sync_socket* s) { return reinterpret_cast<nk_socket_t*>(s); }

	native_sync_listener& as_listener(nk_listener_t* l) { return *reinterpret_cast<native_sync_listener*>(l); }
	const native_sync_listener& as_listener(const nk_listener_t* l) {
		return *reinterpret_cast<const native_sync_listener*>(l);
	}
	nk_listener_t* to_c(native_sync_listener* l) { return reinterpret_cast<nk_listener_t*>(l); }

}

extern "C" nk_status_t nk_addr_create(const char* hostname, int port, nk_addr_type_t type, nk_resolve_method_t method, nk_addr_t** out) {
	return netkit::c::wrap_checked(hostname != nullptr && out != nullptr, [&] {
		auto* a = new addr(std::string(hostname), port, static_cast<netkit::socket::addr_type>(type), static_cast<netkit::socket::resolve_method>(method));
		*out = to_c(a);
	});
}

#ifndef NETKIT_DKP
extern "C" nk_status_t nk_addr_create_path(const char* path, nk_addr_t** out) {
	return netkit::c::wrap_checked(path != nullptr && out != nullptr, [&] {
		auto* a = new addr(std::filesystem::path(path));
		*out = to_c(a);
	});
}
#endif

extern "C" nk_addr_t* nk_addr_clone(const nk_addr_t* a) {
	if (!a) return nullptr;

	try {
		return to_c(new addr(as_addr(a)));
	} catch (...) {
		return nullptr;
	}
}

extern "C" void nk_addr_destroy(nk_addr_t* a) { delete reinterpret_cast<addr*>(a); }
extern "C" bool nk_addr_is_ipv4(const nk_addr_t* a) { return a && as_addr(a).is_ipv4(); }
extern "C" bool nk_addr_is_ipv6(const nk_addr_t* a) { return a && as_addr(a).is_ipv6(); }
extern "C" bool nk_addr_is_file_path(const nk_addr_t* a) { return a && as_addr(a).is_file_path(); }

extern "C" char* nk_addr_get_ip(const nk_addr_t* a) {
	if (!a) return nullptr;
	return netkit::c::dup_string(as_addr(a).get_ip());
}

extern "C" char* nk_addr_get_path(const nk_addr_t* a) {
	if (!a) return nullptr;
	return netkit::c::dup_string(as_addr(a).get_path().string());
}

extern "C" char* nk_addr_get_hostname(const nk_addr_t* a) {
	if (!a) return nullptr;
	return netkit::c::dup_string(as_addr(a).get_hostname());
}

extern "C" int nk_addr_get_port(const nk_addr_t* a) { return a ? as_addr(a).get_port() : 0; }

extern "C" nk_addr_type_t nk_addr_get_type(const nk_addr_t* a) {
	return a ? static_cast<nk_addr_type_t>(as_addr(a).get_type()) : NK_ADDR_HOSTNAME;
}

extern "C" nk_status_t nk_socket_create(const nk_addr_t* a, nk_socket_type_t type, uint32_t opts, nk_socket_t** out) {
	return netkit::c::wrap_checked(a != nullptr && out != nullptr, [&] {
#ifdef NETKIT_DKP
		if (type == NK_SOCK_UDS)
			throw netkit::logic_error("Unix domain sockets are unavailable with NETKIT_DKP");
#endif
		auto* s = new native_sync_socket(as_addr(a), static_cast<netkit::socket::type>(type),
		                                  static_cast<netkit::socket::opt>(opts));
		*out = to_c(static_cast<basic_native_sync_socket*>(s));
	});
}

extern "C" void nk_socket_destroy(nk_socket_t* s) { delete reinterpret_cast<basic_native_sync_socket*>(s); }

extern "C" nk_status_t nk_socket_connect(nk_socket_t* s) {
	return netkit::c::wrap_checked(s != nullptr, [&] { as_socket(s).connect(); });
}

extern "C" nk_status_t nk_socket_send(nk_socket_t* s, const void* buf, size_t len, size_t* out_sent) {
	return netkit::c::wrap_checked(s != nullptr && (buf != nullptr || len == 0), [&] {
		auto sent = as_socket(s).send(buf, len);
		if (out_sent) *out_sent = sent;
	});
}

extern "C" nk_status_t nk_socket_recv(nk_socket_t* s, void* buf, size_t len, size_t* out_received) {
	return netkit::c::wrap_checked(s != nullptr && (buf != nullptr || len == 0), [&] {
		auto received = as_socket(s).recv(buf, len);
		if (out_received) *out_received = received;
	});
}

extern "C" nk_status_t nk_socket_sendto(nk_socket_t* s, const void* buf, size_t len, const nk_addr_t* dest, size_t* out_sent) {
	return netkit::c::wrap_checked(s != nullptr && dest != nullptr && (buf != nullptr || len == 0), [&] {
		auto sent = as_socket(s).sendto(buf, len, as_addr(dest));
		if (out_sent) *out_sent = sent;
	});
}

extern "C" nk_status_t nk_socket_recvfrom(nk_socket_t* s, void* buf, size_t len, size_t* out_received, nk_addr_t** out_from) {
	return netkit::c::wrap_checked(s != nullptr && (buf != nullptr || len == 0), [&] {
		auto [received, from] = as_socket(s).recvfrom(buf, len);
		if (out_received) *out_received = received;
		if (out_from) *out_from = to_c(new addr(from));
	});
}

extern "C" nk_status_t nk_socket_bind(nk_socket_t* s) {
	return netkit::c::wrap_checked(s != nullptr, [&] { as_socket(s).bind(); });
}

extern "C" nk_status_t nk_socket_bind_addr(nk_socket_t* s, const nk_addr_t* a) {
	return netkit::c::wrap_checked(s != nullptr && a != nullptr, [&] { as_socket(s).bind(as_addr(a)); });
}

extern "C" void nk_socket_unbind(nk_socket_t* s) {
	if (s) as_socket(s).unbind();
}

extern "C" bool nk_socket_is_open(const nk_socket_t* s) {
	return s && as_socket(s).is_open();
}

extern "C" void nk_socket_close(nk_socket_t* s) {
	if (s) as_socket(s).close();
}

extern "C" intptr_t nk_socket_native_handle(const nk_socket_t* s) {
	if (!s) return -1;

	try {
		return static_cast<intptr_t>(as_socket(s).native_handle());
	} catch (...) {
		return -1;
	}
}

extern "C" nk_status_t nk_socket_set_opts(nk_socket_t* s, uint32_t opts) {
	return netkit::c::wrap_checked(s != nullptr, [&] { as_socket(s).set_sock_opts(static_cast<netkit::socket::opt>(opts)); });
}

extern "C" nk_status_t nk_socket_get_peer(const nk_socket_t* s, nk_addr_t** out) {
	return netkit::c::wrap_checked(s != nullptr && out != nullptr, [&] {
		*out = to_c(new addr(as_socket(s).get_peer()));
	});
}

extern "C" nk_status_t nk_socket_get_addr(nk_socket_t* s, nk_addr_t** out) {
	return netkit::c::wrap_checked(s != nullptr && out != nullptr, [&] {
		*out = to_c(new addr(as_socket(s).get_addr()));
	});
}

extern "C" nk_status_t nk_listener_create(const nk_addr_t* a, nk_socket_type_t type, uint32_t opts, nk_listener_t** out) {
	return netkit::c::wrap_checked(a != nullptr && out != nullptr, [&] {
#ifdef NETKIT_DKP
		if (type == NK_SOCK_UDS)
			throw netkit::logic_error("Unix domain sockets are unavailable on this platform");
#endif

		auto* l = new native_sync_listener(as_addr(a), static_cast<netkit::socket::type>(type),
		                                    static_cast<netkit::socket::opt>(opts));
		*out = to_c(l);
	});
}

extern "C" void nk_listener_destroy(nk_listener_t* l) {
	delete reinterpret_cast<native_sync_listener*>(l);
}

extern "C" nk_status_t nk_listener_bind(nk_listener_t* l) {
	return netkit::c::wrap_checked(l != nullptr, [&] { as_listener(l).bind(); });
}

extern "C" nk_status_t nk_listener_bind_addr(nk_listener_t* l, const nk_addr_t* a) {
	return netkit::c::wrap_checked(l != nullptr && a != nullptr, [&] { as_listener(l).bind(as_addr(a)); });
}

extern "C" nk_status_t nk_listener_unbind(nk_listener_t* l) {
	return netkit::c::wrap_checked(l != nullptr, [&] { as_listener(l).unbind(); });
}

extern "C" nk_status_t nk_listener_listen(nk_listener_t* l, int backlog) {
	return netkit::c::wrap_checked(l != nullptr, [&] { as_listener(l).listen(backlog); });
}

extern "C" nk_status_t nk_listener_listen_default(nk_listener_t* l) {
	return netkit::c::wrap_checked(l != nullptr, [&] { as_listener(l).listen(); });
}

extern "C" nk_status_t nk_listener_accept(nk_listener_t* l, nk_socket_t** out) {
	return netkit::c::wrap_checked(l != nullptr && out != nullptr, [&] {
		auto sock = as_listener(l).accept();
		*out = to_c(sock.release());
	});
}

extern "C" void nk_listener_close(nk_listener_t* l) {
	if (l) as_listener(l).close();
}

extern "C" nk_status_t nk_listener_get_local_endpoint(const nk_listener_t* l, nk_addr_t** out) {
	return netkit::c::wrap_checked(l != nullptr && out != nullptr, [&] {
		*out = to_c(new addr(as_listener(l).get_local_endpoint()));
	});
}

extern "C" intptr_t nk_listener_native_handle(const nk_listener_t* l) {
	if (!l) return -1;

	try {
		return static_cast<intptr_t>(as_listener(l).native_handle());
	} catch (...) {
		return -1;
	}
}
