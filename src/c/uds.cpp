#include <netkit/definitions.hpp>

#ifndef NETKIT_DKP

#include "internal.hpp"

#include <netkit/c/uds.h>

#include <netkit/socket/addr.hpp>
#include <netkit/socket/native/basic_native_sync_socket.hpp>
#include <netkit/uds/uds_server.hpp>
#include <netkit/uds/uds_stream.hpp>

using netkit::socket::addr;
using netkit::socket::native::basic_native_sync_socket;
using netkit::uds::uds_server;
using netkit::uds::uds_stream;

namespace {
	uds_stream& as_stream(nk_uds_stream_t* s) { return *reinterpret_cast<uds_stream*>(s); }
	const uds_stream& as_stream(const nk_uds_stream_t* s) { return *reinterpret_cast<const uds_stream*>(s); }
	nk_uds_stream_t* to_c(uds_stream* s) { return reinterpret_cast<nk_uds_stream_t*>(s); }

	uds_server& as_server(nk_uds_server_t* s) { return *reinterpret_cast<uds_server*>(s); }
	const uds_server& as_server(const nk_uds_server_t* s) { return *reinterpret_cast<const uds_server*>(s); }
	nk_uds_server_t* to_c(uds_server* s) { return reinterpret_cast<nk_uds_server_t*>(s); }
}

extern "C" nk_status_t nk_uds_stream_create(const nk_addr_t* a, nk_uds_stream_t** out) {
	return netkit::c::wrap_checked(a != nullptr && out != nullptr, [&] {
		*out = to_c(new uds_stream(*reinterpret_cast<const addr*>(a)));
	});
}

extern "C" nk_status_t nk_uds_stream_from_socket(nk_socket_t* sock, nk_uds_stream_t** out) {
	return netkit::c::wrap_checked(sock != nullptr && out != nullptr, [&] {
		std::unique_ptr<basic_native_sync_socket> owned(reinterpret_cast<basic_native_sync_socket*>(sock));
		*out = to_c(new uds_stream(std::move(owned)));
	});
}

extern "C" void nk_uds_stream_destroy(nk_uds_stream_t* s) {
	delete reinterpret_cast<uds_stream*>(s);
}

extern "C" nk_status_t nk_uds_stream_connect(nk_uds_stream_t* s) {
	return netkit::c::wrap_checked(s != nullptr, [&] { as_stream(s).connect(); });
}

extern "C" void nk_uds_stream_close(nk_uds_stream_t* s) {
	if (s) as_stream(s).close();
}

extern "C" bool nk_uds_stream_is_open(const nk_uds_stream_t* s) {
	return s && as_stream(s).is_open();
}

extern "C" nk_status_t nk_uds_stream_peer(const nk_uds_stream_t* s, nk_addr_t** out) {
	return netkit::c::wrap_checked(s != nullptr && out != nullptr, [&] {
		*out = reinterpret_cast<nk_addr_t*>(new addr(as_stream(s).peer()));
	});
}

extern "C" nk_stream_t* nk_uds_stream_as_stream(nk_uds_stream_t* s) {
	if (!s) return nullptr;
	auto* base = static_cast<netkit::stream::basic_stream*>(&as_stream(s));
	return reinterpret_cast<nk_stream_t*>(base);
}

extern "C" nk_status_t nk_uds_server_create(const nk_addr_t* a, nk_uds_server_t** out) {
	return netkit::c::wrap_checked(a != nullptr && out != nullptr, [&] {
		*out = to_c(new uds_server(*reinterpret_cast<const addr*>(a)));
	});
}

extern "C" void nk_uds_server_destroy(nk_uds_server_t* s) {
	delete reinterpret_cast<uds_server*>(s);
}

extern "C" nk_status_t nk_uds_server_bind(nk_uds_server_t* s) {
	return netkit::c::wrap_checked(s != nullptr, [&] { as_server(s).bind(); });
}

extern "C" nk_status_t nk_uds_server_listen(nk_uds_server_t* s, int backlog) {
	return netkit::c::wrap_checked(s != nullptr, [&] { as_server(s).listen(backlog); });
}

extern "C" nk_status_t nk_uds_server_listen_default(nk_uds_server_t* s) {
	return netkit::c::wrap_checked(s != nullptr, [&] { as_server(s).listen(); });
}

extern "C" nk_status_t nk_uds_server_accept(nk_uds_server_t* s, nk_uds_stream_t** out) {
	return netkit::c::wrap_checked(s != nullptr && out != nullptr, [&] {
		auto stream = as_server(s).accept();
		*out = to_c(stream.release());
	});
}

extern "C" void nk_uds_server_close(nk_uds_server_t* s) {
	if (s) as_server(s).close();
}

extern "C" nk_status_t nk_uds_server_get_local_endpoint(const nk_uds_server_t* s, nk_addr_t** out) {
	return netkit::c::wrap_checked(s != nullptr && out != nullptr, [&] {
		*out = reinterpret_cast<nk_addr_t*>(new addr(as_server(s).get_local_endpoint()));
	});
}

#endif
