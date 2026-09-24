/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file http_server.h
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief C bindings for netkit's synchronous HTTP server
 */
#pragma once

#ifdef NETKIT_HTTP

#include <netkit/c/common.h>
#include <netkit/c/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum nk_http_redirect_type {
	NK_REDIRECT_PERMANENT = 0,
	NK_REDIRECT_TEMPORARY = 1
} nk_http_redirect_type_t;

NETKIT_API nk_status_t nk_http_server_settings_create(nk_http_server_settings_t** out);
NETKIT_API void nk_http_server_settings_destroy(nk_http_server_settings_t* settings);

NETKIT_API void nk_http_server_settings_set_port(nk_http_server_settings_t* settings, int port);
NETKIT_API void nk_http_server_settings_set_enable_session(nk_http_server_settings_t* settings, bool enable);
NETKIT_API void nk_http_server_settings_set_session_directory(nk_http_server_settings_t* settings, const char* dir);
NETKIT_API void nk_http_server_settings_set_session_cookie_name(nk_http_server_settings_t* settings, const char* name);
NETKIT_API void nk_http_server_settings_set_session_is_secure(nk_http_server_settings_t* settings, bool secure);
NETKIT_API void nk_http_server_settings_add_blacklisted_ip(nk_http_server_settings_t* settings, const char* ip);
NETKIT_API void nk_http_server_settings_add_associated_session_cookie(nk_http_server_settings_t* settings, const char* cookie_name);
NETKIT_API void nk_http_server_settings_set_trust_x_forwarded_for(nk_http_server_settings_t* settings, bool trust);
NETKIT_API void nk_http_server_settings_set_max_connections(nk_http_server_settings_t* settings, int max_connections);

NETKIT_API const char* nk_http_request_endpoint(const nk_http_request_t* request);
NETKIT_API const char* nk_http_request_method(const nk_http_request_t* request);
NETKIT_API const char* nk_http_request_content_type(const nk_http_request_t* request);
NETKIT_API const char* nk_http_request_ip_address(const nk_http_request_t* request);
NETKIT_API const char* nk_http_request_user_agent(const nk_http_request_t* request);
NETKIT_API unsigned int nk_http_request_version(const nk_http_request_t* request);
NETKIT_API const char* nk_http_request_session_id(const nk_http_request_t* request);

NETKIT_API bool nk_http_request_query(const nk_http_request_t* request, const char* key, const char** out_value);
NETKIT_API bool nk_http_request_header(const nk_http_request_t* request, const char* name, const char** out_value);
NETKIT_API bool nk_http_request_session(const nk_http_request_t* request, const char* key, const char** out_value);

NETKIT_API size_t nk_http_request_cookie_count(const nk_http_request_t* request);
NETKIT_API bool nk_http_request_cookie_at(const nk_http_request_t* request, size_t index, const char** out_name, const char** out_value);

NETKIT_API nk_body_t* nk_http_request_body(const nk_http_request_t* request);
NETKIT_API nk_status_t nk_http_server_response_create(nk_http_server_response_t** out);
NETKIT_API void nk_http_server_response_destroy(nk_http_server_response_t* response);
NETKIT_API void nk_http_server_response_set_status(nk_http_server_response_t* response, int status);
NETKIT_API void nk_http_server_response_set_content_type(nk_http_server_response_t* response, const char* content_type);
NETKIT_API void nk_http_server_response_set_allow_origin(nk_http_server_response_t* response, const char* origin);
NETKIT_API void nk_http_server_response_set_location(nk_http_server_response_t* response, const char* location);
NETKIT_API void nk_http_server_response_set_redirect_type(nk_http_server_response_t* response, nk_http_redirect_type_t type);
NETKIT_API void nk_http_server_response_set_stop(nk_http_server_response_t* response, bool stop);
NETKIT_API void nk_http_server_response_add_header(nk_http_server_response_t* response, const char* name, const char* value);
NETKIT_API void nk_http_server_response_set_session(nk_http_server_response_t* response, const char* key, const char* value);
NETKIT_API void nk_http_server_response_delete_cookie(nk_http_server_response_t* response, const char* name);
NETKIT_API void nk_http_server_response_add_cookie(nk_http_server_response_t* response, const char* name, const char* value, long long expires_unix_millis, const char* path, const char* domain, const char* same_site, bool http_only, bool secure);
NETKIT_API void nk_http_server_response_set_body(nk_http_server_response_t* response, nk_body_t* body);

typedef nk_http_server_response_t* (*nk_http_request_callback_t)(const nk_http_request_t* request, void* user_data);

NETKIT_API nk_status_t nk_http_server_create(nk_http_server_settings_t* settings, nk_http_request_callback_t callback, void* user_data, nk_http_server_t** out);
NETKIT_API void nk_http_server_destroy(nk_http_server_t* server);
NETKIT_API nk_status_t nk_http_server_run(nk_http_server_t* server);
NETKIT_API void nk_http_server_stop(nk_http_server_t* server);

#ifdef __cplusplus
}
#endif

#endif /* NETKIT_HTTP */
