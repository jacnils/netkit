#include "internal.hpp"

#include <netkit/c/stream.h>

#include <netkit/body/basic_body.hpp>
#include <netkit/stream/basic_stream.hpp>

using netkit::stream::basic_stream;
using netkit::stream::stream_result;
using netkit::stream::stream_status;

namespace {
	basic_stream& as_stream(nk_stream_t* s) { return *reinterpret_cast<basic_stream*>(s); }
	const basic_stream& as_stream(const nk_stream_t* s) { return *reinterpret_cast<const basic_stream*>(s); }

	nk_stream_result_t to_c(const stream_result& r) {
		return nk_stream_result_t{r.bytes, static_cast<nk_stream_status_t>(r.status)};
	}

	nk_stream_result_t error_result() { return nk_stream_result_t{0, NK_STREAM_ERROR}; }
}

extern "C" nk_stream_result_t nk_stream_read(nk_stream_t* s, void* buffer, size_t len) {
	if (!s || (!buffer && len != 0))
		return error_result();

	try {
		return to_c(as_stream(s).read(buffer, len));
	} catch (const std::exception& e) {
		netkit::c::set_last_error(e.what());
		return error_result();
	}
}

extern "C" nk_stream_result_t nk_stream_write(nk_stream_t* s, const void* buffer, size_t len) {
	if (!s || (!buffer && len != 0))
		return error_result();

	try {
		return to_c(as_stream(s).write(buffer, len));
	} catch (const std::exception& e) {
		netkit::c::set_last_error(e.what());
		return error_result();
	}
}

extern "C" nk_stream_result_t nk_stream_write_all(nk_stream_t* s, const void* buffer, size_t len) {
	if (!s || (!buffer && len != 0))
		return error_result();

	try {
		return to_c(as_stream(s).write_all(buffer, len));
	} catch (const std::exception& e) {
		netkit::c::set_last_error(e.what());
		return error_result();
	}
}

extern "C" nk_stream_result_t nk_stream_write_all_body(nk_stream_t* s, nk_body_t* body) {
	if (!s || !body) return error_result();
	try {
		return to_c(as_stream(s).write_all(*reinterpret_cast<netkit::body::basic_body*>(body)));
	} catch (const std::exception& e) {
		netkit::c::set_last_error(e.what());
		return error_result();
	}
}

extern "C" void nk_stream_close(nk_stream_t* s) {
	if (s) as_stream(s).close();
}

extern "C" bool nk_stream_is_open(const nk_stream_t* s) {
	return s && as_stream(s).is_open();
}

extern "C" nk_status_t nk_stream_read_all(nk_stream_t* s, size_t max_bytes, char** out_data, size_t* out_len) {
	return netkit::c::wrap_checked(s != nullptr && out_data != nullptr && out_len != nullptr, [&] {
		auto data = as_stream(s).read_all(max_bytes == 0 ? (16 * 1024 * 1024) : max_bytes);
		*out_data = netkit::c::dup_bytes(reinterpret_cast<const char*>(data.data()), data.size());
		*out_len = data.size();
	});
}

extern "C" nk_status_t nk_stream_get_addr(nk_stream_t* s, nk_addr_t** out) {
	return netkit::c::wrap_checked(s != nullptr && out != nullptr, [&] {
		auto addr_opt = as_stream(s).get_addr();
		*out = addr_opt ? reinterpret_cast<nk_addr_t*>(new netkit::socket::addr(*addr_opt)) : nullptr;
	});
}
