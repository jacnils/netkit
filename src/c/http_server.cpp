#ifdef NETKIT_HTTP

#include "internal.hpp"

#include <netkit/c/http_server.h>

#include <netkit/body/buffer_body.hpp>
#include <netkit/http/server_predefined.hpp>
#include <netkit/http/sync_server.hpp>

using netkit::http::server::cookie;
using netkit::http::server::request;
using netkit::http::server::response;
using netkit::http::server::server_settings;
using netkit::http::server::sync_server;

namespace {
	inline server_settings& as_settings(nk_http_server_settings_t* s) { return *reinterpret_cast<server_settings*>(s); }
	inline nk_http_server_settings_t* to_c(server_settings* s) { return reinterpret_cast<nk_http_server_settings_t*>(s); }

	inline const request& as_request(const nk_http_request_t* r) { return *reinterpret_cast<const request*>(r); }

	inline response& as_response(nk_http_server_response_t* r) { return *reinterpret_cast<response*>(r); }
	inline nk_http_server_response_t* to_c(response* r) { return reinterpret_cast<nk_http_server_response_t*>(r); }

	inline sync_server<>& as_server(nk_http_server_t* s) { return *reinterpret_cast<sync_server<>*>(s); }

	std::string safe(const char* s) { return s ? std::string(s) : std::string(); }
} // namespace

/* ------------------------------------------------------------------------------------------- */
/* settings                                                                                     */
/* ------------------------------------------------------------------------------------------- */

extern "C" nk_status_t nk_http_server_settings_create(nk_http_server_settings_t** out) {
	return netkit::c::wrap_checked(out != nullptr, [&] { *out = to_c(new server_settings()); });
}

extern "C" void nk_http_server_settings_destroy(nk_http_server_settings_t* s) {
	delete reinterpret_cast<server_settings*>(s);
}

extern "C" void nk_http_server_settings_set_port(nk_http_server_settings_t* s, int port) {
	if (s) as_settings(s).port = port;
}
extern "C" void nk_http_server_settings_set_enable_session(nk_http_server_settings_t* s, bool enable) {
	if (s) as_settings(s).enable_session = enable;
}
extern "C" void nk_http_server_settings_set_session_directory(nk_http_server_settings_t* s, const char* dir) {
	if (s && dir) as_settings(s).session_directory = dir;
}
extern "C" void nk_http_server_settings_set_session_cookie_name(nk_http_server_settings_t* s, const char* name) {
	if (s && name) as_settings(s).session_cookie_name = name;
}
extern "C" void nk_http_server_settings_set_session_is_secure(nk_http_server_settings_t* s, bool secure) {
	if (s) as_settings(s).session_is_secure = secure;
}
extern "C" void nk_http_server_settings_add_blacklisted_ip(nk_http_server_settings_t* s, const char* ip) {
	if (s && ip) as_settings(s).blacklisted_ips.emplace_back(ip);
}
extern "C" void nk_http_server_settings_add_associated_session_cookie(nk_http_server_settings_t* s, const char* name) {
	if (s && name) as_settings(s).associated_session_cookies.emplace_back(name);
}
extern "C" void nk_http_server_settings_set_trust_x_forwarded_for(nk_http_server_settings_t* s, bool trust) {
	if (s) as_settings(s).trust_x_forwarded_for = trust;
}
extern "C" void nk_http_server_settings_set_max_connections(nk_http_server_settings_t* s, int max_connections) {
	if (s) as_settings(s).max_connections = max_connections;
}

/* ------------------------------------------------------------------------------------------- */
/* request                                                                                      */
/* ------------------------------------------------------------------------------------------- */

extern "C" const char* nk_http_request_endpoint(const nk_http_request_t* r) {
	return r ? as_request(r).endpoint.c_str() : "";
}
extern "C" const char* nk_http_request_method(const nk_http_request_t* r) {
	return r ? as_request(r).method.c_str() : "";
}
extern "C" const char* nk_http_request_content_type(const nk_http_request_t* r) {
	return r ? as_request(r).content_type.c_str() : "";
}
extern "C" const char* nk_http_request_ip_address(const nk_http_request_t* r) {
	return r ? as_request(r).ip_address.c_str() : "";
}
extern "C" const char* nk_http_request_user_agent(const nk_http_request_t* r) {
	return r ? as_request(r).user_agent.c_str() : "";
}
extern "C" unsigned int nk_http_request_version(const nk_http_request_t* r) { return r ? as_request(r).version : 0; }
extern "C" const char* nk_http_request_session_id(const nk_http_request_t* r) {
	return r ? as_request(r).session_id.c_str() : "";
}

extern "C" bool nk_http_request_query(const nk_http_request_t* r, const char* key, const char** out_value) {
	if (!r || !key || !out_value) return false;
	const auto& q = as_request(r).query;
	const auto it = q.find(key);
	if (it == q.end()) return false;
	*out_value = it->second.c_str();
	return true;
}

extern "C" bool nk_http_request_header(const nk_http_request_t* r, const char* name, const char** out_value) {
	if (!r || !name || !out_value) return false;
	const auto& h = as_request(r).headers;
	const auto it = h.find(name);
	if (it == h.end()) return false;
	*out_value = it->second.c_str();
	return true;
}

extern "C" bool nk_http_request_session(const nk_http_request_t* r, const char* key, const char** out_value) {
	if (!r || !key || !out_value) return false;
	const auto& s = as_request(r).session;
	const auto it = s.find(key);
	if (it == s.end()) return false;
	*out_value = it->second.c_str();
	return true;
}

extern "C" size_t nk_http_request_cookie_count(const nk_http_request_t* r) {
	return r ? as_request(r).cookies.size() : 0;
}

extern "C" bool nk_http_request_cookie_at(const nk_http_request_t* r, size_t index, const char** out_name,
                                           const char** out_value) {
	if (!r || index >= as_request(r).cookies.size()) return false;
	const auto& c = as_request(r).cookies[index];
	if (out_name) *out_name = c.name.c_str();
	if (out_value) *out_value = c.value.c_str();
	return true;
}

extern "C" nk_body_t* nk_http_request_body(const nk_http_request_t* r) {
	if (!r) return nullptr;
	return reinterpret_cast<nk_body_t*>(as_request(r).body.get());
}

/* ------------------------------------------------------------------------------------------- */
/* response                                                                                     */
/* ------------------------------------------------------------------------------------------- */

extern "C" nk_status_t nk_http_server_response_create(nk_http_server_response_t** out) {
	return netkit::c::wrap_checked(out != nullptr, [&] {
		auto* resp = new response();
		resp->body = std::make_unique<netkit::body::buffer_body>();
		*out = to_c(resp);
	});
}

extern "C" void nk_http_server_response_destroy(nk_http_server_response_t* r) { delete reinterpret_cast<response*>(r); }

extern "C" void nk_http_server_response_set_status(nk_http_server_response_t* r, int status) {
	if (r) as_response(r).http_status = status;
}
extern "C" void nk_http_server_response_set_content_type(nk_http_server_response_t* r, const char* content_type) {
	if (r && content_type) as_response(r).content_type = content_type;
}
extern "C" void nk_http_server_response_set_allow_origin(nk_http_server_response_t* r, const char* origin) {
	if (r) as_response(r).allow_origin = safe(origin);
}
extern "C" void nk_http_server_response_set_location(nk_http_server_response_t* r, const char* location) {
	if (r) as_response(r).location = safe(location);
}
extern "C" void nk_http_server_response_set_redirect_type(nk_http_server_response_t* r, nk_http_redirect_type_t type) {
	if (r) as_response(r).redirection = static_cast<netkit::http::server::redirect_type>(type);
}
extern "C" void nk_http_server_response_set_stop(nk_http_server_response_t* r, bool stop) {
	if (r) as_response(r).stop = stop;
}
extern "C" void nk_http_server_response_add_header(nk_http_server_response_t* r, const char* name, const char* value) {
	if (r && name && value) as_response(r).headers.push_back({name, value});
}
extern "C" void nk_http_server_response_set_session(nk_http_server_response_t* r, const char* key, const char* value) {
	if (r && key && value) as_response(r).session[key] = value;
}
extern "C" void nk_http_server_response_delete_cookie(nk_http_server_response_t* r, const char* name) {
	if (r && name) as_response(r).delete_cookies.emplace_back(name);
}

extern "C" void nk_http_server_response_add_cookie(nk_http_server_response_t* r, const char* name, const char* value,
                                                     long long expires_unix_millis, const char* path,
                                                     const char* domain, const char* same_site, bool http_only,
                                                     bool secure) {
	if (!r || !name) return;
	cookie c;
	c.name = name;
	c.value = safe(value);
	c.expires = expires_unix_millis;
	if (path) c.path = path;
	if (domain) c.domain = domain;
	if (same_site) c.same_site = same_site;
	c.http_only = http_only;
	c.secure = secure;
	as_response(r).cookies.push_back(std::move(c));
}

extern "C" void nk_http_server_response_set_body(nk_http_server_response_t* r, nk_body_t* body) {
	if (!r) return;
	as_response(r).body.reset(reinterpret_cast<netkit::body::basic_body*>(body));
}

/* ------------------------------------------------------------------------------------------- */
/* server                                                                                       */
/* ------------------------------------------------------------------------------------------- */

extern "C" nk_status_t nk_http_server_create(nk_http_server_settings_t* settings, nk_http_request_callback_t callback,
                                              void* user_data, nk_http_server_t** out) {
	return netkit::c::wrap_checked(settings != nullptr && callback != nullptr && out != nullptr, [&] {
		std::unique_ptr<server_settings> settings_owner(reinterpret_cast<server_settings*>(settings));

		auto cpp_callback = [callback, user_data](const request& req) -> response {
			nk_http_server_response_t* c_resp = callback(reinterpret_cast<const nk_http_request_t*>(&req), user_data);

			if (!c_resp) {
				response def;
				def.http_status = 200;
				def.body = std::make_unique<netkit::body::buffer_body>();
				return def;
			}

			auto* resp_ptr = reinterpret_cast<response*>(c_resp);
			response result = std::move(*resp_ptr);
			delete resp_ptr;
			return result;
		};

		auto* srv = new sync_server<>(std::move(*settings_owner), cpp_callback);
		*out = reinterpret_cast<nk_http_server_t*>(srv);
	});
}

extern "C" void nk_http_server_destroy(nk_http_server_t* s) {
	if (!s) return;
	as_server(s).stop();
	delete reinterpret_cast<sync_server<>*>(s);
}

extern "C" nk_status_t nk_http_server_run(nk_http_server_t* s) {
	return netkit::c::wrap_checked(s != nullptr, [&] { as_server(s).run(); });
}

extern "C" void nk_http_server_stop(nk_http_server_t* s) {
	if (s) as_server(s).stop();
}

#endif /* NETKIT_HTTP */
