#include "internal.hpp"

#include <netkit/c/http.h>

#include <netkit/http/client.hpp>
#include <netkit/socket/addr.hpp>

using netkit::http::client;
using netkit::http::headers;
using netkit::http::response;

namespace {
	inline headers& as_headers(nk_http_headers_t* h) { return *reinterpret_cast<headers*>(h); }
	inline const headers& as_headers(const nk_http_headers_t* h) { return *reinterpret_cast<const headers*>(h); }
	inline nk_http_headers_t* to_c(headers* h) { return reinterpret_cast<nk_http_headers_t*>(h); }

	inline client& as_client(nk_http_client_t* c) { return *reinterpret_cast<client*>(c); }
	inline nk_http_client_t* to_c(client* c) { return reinterpret_cast<nk_http_client_t*>(c); }

	inline response& as_response(nk_http_response_t* r) { return *reinterpret_cast<response*>(r); }
	inline const response& as_response(const nk_http_response_t* r) { return *reinterpret_cast<const response*>(r); }
	inline nk_http_response_t* to_c(response* r) { return reinterpret_cast<nk_http_response_t*>(r); }

	netkit::http::method to_cpp_method(nk_http_method_t m) {
		using netkit::http::method;
		switch (m) {
			case NK_HTTP_GET: return method::method_get;
			case NK_HTTP_HEAD: return method::method_head;
			case NK_HTTP_POST: return method::method_post;
			case NK_HTTP_PUT: return method::method_put;
			case NK_HTTP_DELETE: return method::method_delete;
			case NK_HTTP_CONNECT: return method::method_connect;
			case NK_HTTP_OPTIONS: return method::method_options;
			case NK_HTTP_TRACE: return method::method_trace;
			case NK_HTTP_PATCH: return method::method_patch;
			default: return method::method_undefined;
		}
	}

	/// client::request() takes `const std::unique_ptr<body::basic_body>&` without taking ownership:
	/// this wraps a borrowed nk_body_t* for the duration of one call, then releases it so the
	/// object outlives the call (it still belongs to the C caller).
	struct borrowed_body {
		std::unique_ptr<netkit::body::basic_body> ptr;
		explicit borrowed_body(nk_body_t* b) : ptr(reinterpret_cast<netkit::body::basic_body*>(b)) {}
		~borrowed_body() { ptr.release(); }
	};

	nk_status_t do_request(client& c, const std::string& method, const char* path, nk_body_t* body,
	                        const nk_http_headers_t* h, nk_http_response_t** out) {
		return netkit::c::wrap_checked(path != nullptr && out != nullptr, [&] {
			borrowed_body b(body);
			headers hdrs = h ? as_headers(h) : headers{};
			auto resp = c.request(method, path, b.ptr, hdrs);
			*out = to_c(new response(std::move(resp)));
		});
	}
} // namespace

/* ------------------------------------------------------------------------------------------- */
/* headers                                                                                      */
/* ------------------------------------------------------------------------------------------- */

extern "C" nk_status_t nk_http_headers_create(nk_http_headers_t** out) {
	return netkit::c::wrap_checked(out != nullptr, [&] { *out = to_c(new headers()); });
}

extern "C" void nk_http_headers_destroy(nk_http_headers_t* h) { delete reinterpret_cast<headers*>(h); }

extern "C" nk_status_t nk_http_headers_add(nk_http_headers_t* h, const char* name, const char* value) {
	return netkit::c::wrap_checked(h != nullptr && name != nullptr && value != nullptr,
	                                [&] { as_headers(h).add(std::string(name), std::string(value)); });
}

extern "C" size_t nk_http_headers_count(const nk_http_headers_t* h) { return h ? as_headers(h).size() : 0; }

extern "C" bool nk_http_headers_at(const nk_http_headers_t* h, size_t index, char** out_name, char** out_value) {
	if (!h || index >= as_headers(h).size()) return false;
	const auto& entry = as_headers(h)[index];
	if (out_name) *out_name = netkit::c::dup_string(entry.name.value());
	if (out_value) *out_value = netkit::c::dup_string(entry.value);
	return true;
}

extern "C" bool nk_http_headers_find(const nk_http_headers_t* h, const char* name, char** out_value) {
	if (!h || !name || !out_value) return false;
	const auto* entry = as_headers(h).find(std::string(name));
	if (!entry) return false;
	*out_value = netkit::c::dup_string(entry->value);
	return true;
}

/* ------------------------------------------------------------------------------------------- */
/* client                                                                                       */
/* ------------------------------------------------------------------------------------------- */

extern "C" nk_status_t nk_http_client_create(const nk_addr_t* a, nk_http_scheme_t scheme, nk_http_client_t** out) {
	return netkit::c::wrap_checked(a != nullptr && out != nullptr, [&] {
		netkit::http::scheme cpp_scheme;
		if (scheme == NK_HTTP_HTTPS) {
#ifdef NETKIT_SSL
			cpp_scheme = netkit::http::scheme::https;
#else
			throw netkit::logic_error("HTTPS requires netkit to be built with WolfSSL support");
#endif
		} else {
			cpp_scheme = netkit::http::scheme::http;
		}
		*out = to_c(new client(*reinterpret_cast<const netkit::socket::addr*>(a), cpp_scheme));
	});
}

extern "C" void nk_http_client_destroy(nk_http_client_t* c) { delete reinterpret_cast<client*>(c); }

extern "C" void nk_http_client_set_user_agent(nk_http_client_t* c, const char* ua) {
	if (c && ua) as_client(c).set_user_agent(ua);
}

extern "C" void nk_http_client_set_accept(nk_http_client_t* c, const char* accept) {
	if (c && accept) as_client(c).set_accept(accept);
}

extern "C" void nk_http_client_set_content_type(nk_http_client_t* c, const char* content_type) {
	if (c && content_type) as_client(c).set_content_type(content_type);
}

extern "C" void nk_http_client_set_close(nk_http_client_t* c, bool close) {
	if (c) as_client(c).set_close(close);
}

extern "C" nk_status_t nk_http_client_request(nk_http_client_t* c, nk_http_method_t method, const char* path,
                                               nk_body_t* body, const nk_http_headers_t* h, nk_http_response_t** out) {
	if (!c) return NK_ERR_INVALID_ARGUMENT;
	return do_request(as_client(c), netkit::http::get_method_string(to_cpp_method(method)), path, body, h, out);
}

extern "C" nk_status_t nk_http_client_request_custom(nk_http_client_t* c, const char* method, const char* path,
                                                      nk_body_t* body, const nk_http_headers_t* h,
                                                      nk_http_response_t** out) {
	if (!c || !method) return NK_ERR_INVALID_ARGUMENT;
	return do_request(as_client(c), method, path, body, h, out);
}

extern "C" nk_status_t nk_http_client_get(nk_http_client_t* c, const char* path, const nk_http_headers_t* h,
                                           nk_http_response_t** out) {
	return nk_http_client_request(c, NK_HTTP_GET, path, nullptr, h, out);
}

extern "C" nk_status_t nk_http_client_post(nk_http_client_t* c, const char* path, nk_body_t* body,
                                            const nk_http_headers_t* h, nk_http_response_t** out) {
	return nk_http_client_request(c, NK_HTTP_POST, path, body, h, out);
}

extern "C" nk_status_t nk_http_client_put(nk_http_client_t* c, const char* path, nk_body_t* body,
                                           const nk_http_headers_t* h, nk_http_response_t** out) {
	return nk_http_client_request(c, NK_HTTP_PUT, path, body, h, out);
}

extern "C" nk_status_t nk_http_client_patch(nk_http_client_t* c, const char* path, nk_body_t* body,
                                             const nk_http_headers_t* h, nk_http_response_t** out) {
	return nk_http_client_request(c, NK_HTTP_PATCH, path, body, h, out);
}

/* ------------------------------------------------------------------------------------------- */
/* response                                                                                     */
/* ------------------------------------------------------------------------------------------- */

extern "C" void nk_http_response_destroy(nk_http_response_t* r) { delete reinterpret_cast<response*>(r); }

extern "C" int nk_http_response_status_code(const nk_http_response_t* r) { return r ? as_response(r).status_code : 0; }

extern "C" const nk_http_headers_t* nk_http_response_headers(const nk_http_response_t* r) {
	if (!r) return nullptr;
	return reinterpret_cast<const nk_http_headers_t*>(&as_response(r).headers);
}

extern "C" nk_body_t* nk_http_response_take_body(nk_http_response_t* r) {
	if (!r) return nullptr;
	return reinterpret_cast<nk_body_t*>(as_response(r).body.release());
}
