#include "internal.hpp"

#include <netkit/c/body.h>

#include <netkit/body/basic_body.hpp>
#include <netkit/body/buffer_body.hpp>
#include <netkit/body/chunked_body.hpp>
#include <netkit/body/file_body.hpp>
#include <netkit/body/stream_body.hpp>
#include <netkit/stream/basic_stream.hpp>

using netkit::body::basic_body;
using netkit::body::buffer_body;
using netkit::body::chunked_body;
using netkit::body::file_body;
using netkit::body::stream_body;

// these are kinda retarded do we actually need them?
// i guess it makes the rest of the code slightly less nasty
namespace {
	basic_body& as_body(nk_body_t* b) {
		return *reinterpret_cast<basic_body*>(b);
	}
	const basic_body& as_body(const nk_body_t* b) {
		return *reinterpret_cast<const basic_body*>(b);
	}
	nk_body_t* to_c(basic_body* b) {
		return reinterpret_cast<nk_body_t*>(b);
	}

	netkit::stream::basic_stream& as_stream(nk_stream_t* s) {
		return *reinterpret_cast<netkit::stream::basic_stream*>(s);
	}
}

extern "C" nk_status_t nk_body_create_buffer(nk_body_t** out) {
	return netkit::c::wrap_checked(out != nullptr, [&] { *out = to_c(new buffer_body()); });
}

extern "C" nk_status_t nk_body_create_buffer_from(const char* data, size_t len, nk_body_t** out) {
	return netkit::c::wrap_checked(out != nullptr && (data != nullptr || len == 0), [&] {
		*out = to_c(new buffer_body(std::string(data, len)));
	});
}

extern "C" nk_status_t nk_body_create_file(const char* path, nk_body_t** out) {
	return netkit::c::wrap_checked(path != nullptr && out != nullptr, [&] { *out = to_c(new file_body(std::string(path))); });
}

extern "C" bool nk_body_file_is_open(const nk_body_t* b) {
	if (!b) return false;
	if (const auto* fb = dynamic_cast<const file_body*>(&as_body(b))) return fb->is_open();
	return false;
}

extern "C" nk_status_t nk_body_create_stream(nk_stream_t* stream, bool has_length, size_t length, const char* initial, size_t initial_len, nk_body_t** out) {
	return netkit::c::wrap_checked(stream != nullptr && out != nullptr && (initial != nullptr || initial_len == 0), [&] {
		std::optional<std::size_t> len = has_length ? std::optional<std::size_t>(length) : std::nullopt;
		std::string init = initial ? std::string(initial, initial_len) : std::string();
		*out = to_c(new stream_body(as_stream(stream), len, std::move(init)));
	});
}

extern "C" nk_status_t nk_body_create_chunked(nk_stream_t* stream, const char* initial, size_t initial_len, nk_body_t** out) {
	return netkit::c::wrap_checked(stream != nullptr && out != nullptr && (initial != nullptr || initial_len == 0), [&] {
		std::string init = initial ? std::string(initial, initial_len) : std::string();
		*out = to_c(new chunked_body(as_stream(stream), std::move(init)));
	});
}

extern "C" void nk_body_destroy(nk_body_t* b) {
	delete reinterpret_cast<basic_body*>(b);
}

extern "C" nk_read_result_t nk_body_read(nk_body_t* b, char* buffer, size_t max_bytes) {
	if (!b || (!buffer && max_bytes != 0)) return nk_read_result_t{NK_READ_ERROR, 0};
	auto res = as_body(b).read(buffer, max_bytes);
	return nk_read_result_t{static_cast<nk_read_status_t>(res.get_status()), res.get_bytes_read()};
}

extern "C" nk_status_t nk_body_read_all(nk_body_t* b, bool has_max_size, size_t max_size, char** out_data, size_t* out_len) {
	return netkit::c::wrap_checked(b != nullptr && out_data != nullptr && out_len != nullptr, [&] {
		std::optional<std::size_t> size = has_max_size ? std::optional<std::size_t>(max_size) : std::nullopt;
		auto data = as_body(b).read_all(size);
		*out_data = netkit::c::dup_bytes(data.data(), data.size());
		*out_len = data.size();
	});
}

extern "C" bool nk_body_size(const nk_body_t* b, size_t* out_size) {
	if (!b) return false;
	auto sz = as_body(b).size();
	if (!sz) return false;
	if (out_size) *out_size = *sz;
	return true;
}

extern "C" bool nk_body_empty(const nk_body_t* b) {
	return b && as_body(b).empty();
}

extern "C" bool nk_body_rewind(nk_body_t* b) {
	return b && as_body(b).rewind();
}

extern "C" nk_status_t nk_body_buffer_set(nk_body_t* b, const char* data, size_t len) {
	return netkit::c::wrap_checked(b != nullptr && (data != nullptr || len == 0), [&] {
		auto* bb = dynamic_cast<buffer_body*>(&as_body(b));
		if (!bb) throw netkit::logic_error("nk_body_buffer_set: body is not a buffer body");
		bb->set(std::string(data, len));
	});
}

extern "C" nk_status_t nk_body_buffer_append(nk_body_t* b, const char* data, size_t len) {
	return netkit::c::wrap_checked(b != nullptr && (data != nullptr || len == 0), [&] {
		auto* bb = dynamic_cast<buffer_body*>(&as_body(b));
		if (!bb) throw netkit::logic_error("nk_body_buffer_append: body is not a buffer body");
		bb->append(std::string(data, len));
	});
}
