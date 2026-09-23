#include "internal.hpp"

#include <netkit/c/udp.h>

#include <netkit/socket/addr.hpp>
#include <netkit/udp/udp_datagram.hpp>

using netkit::socket::addr;
using netkit::udp::udp_datagram;

namespace {
	udp_datagram& as_dgram(nk_udp_datagram_t* d) { return *reinterpret_cast<udp_datagram*>(d); }
	nk_udp_datagram_t* to_c(udp_datagram* d) { return reinterpret_cast<nk_udp_datagram_t*>(d); }
}

extern "C" nk_status_t nk_udp_datagram_create(const nk_addr_t* a, nk_udp_datagram_t** out) {
	return netkit::c::wrap_checked(a != nullptr && out != nullptr, [&] {
		*out = to_c(new udp_datagram(*reinterpret_cast<const addr*>(a)));
	});
}

extern "C" void nk_udp_datagram_destroy(nk_udp_datagram_t* d) {
	delete reinterpret_cast<udp_datagram*>(d);
}

extern "C" nk_status_t nk_udp_datagram_bind(nk_udp_datagram_t* d) {
	return netkit::c::wrap_checked(d != nullptr, [&] { as_dgram(d).bind(); });
}

extern "C" nk_status_t nk_udp_datagram_send_to(nk_udp_datagram_t* d, const void* buffer, size_t len, const nk_addr_t* dest, size_t* out_sent) {
	return netkit::c::wrap_checked(d != nullptr && dest != nullptr && (buffer != nullptr || len == 0), [&] {
		auto span = std::as_bytes(std::span(static_cast<const char*>(buffer), len));
		auto sent = as_dgram(d).send_to(span, *reinterpret_cast<const addr*>(dest));
		if (out_sent) *out_sent = sent;
	});
}

extern "C" nk_status_t nk_udp_datagram_recv_from(nk_udp_datagram_t* d, void* buffer, size_t len, size_t* out_received, nk_addr_t** out_from) {
	return netkit::c::wrap_checked(d != nullptr && (buffer != nullptr || len == 0), [&] {
		auto span = std::as_writable_bytes(std::span(static_cast<char*>(buffer), len));
		auto [received, from] = as_dgram(d).recv_from(span);

		if (out_received) *out_received = received;
		if (out_from) *out_from = reinterpret_cast<nk_addr_t*>(new addr(std::move(from)));
	});
}

extern "C" void nk_udp_datagram_close(nk_udp_datagram_t* d) {
	if (d)
		as_dgram(d).close();
}

extern "C" bool nk_udp_datagram_is_open(const nk_udp_datagram_t* d) {
	return d && reinterpret_cast<const udp_datagram*>(d)->is_open();
}
