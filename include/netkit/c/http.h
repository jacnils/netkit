/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file http.h
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief C bindings for netkit::http::client
 */
#pragma once

#include <netkit/c/common.h>
#include <netkit/c/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum nk_http_scheme {
	NK_HTTP_HTTP = 0,
	NK_HTTP_HTTPS = 1
} nk_http_scheme_t;

typedef enum nk_http_method {
	NK_HTTP_GET = 0,
	NK_HTTP_HEAD = 1,
	NK_HTTP_POST = 2,
	NK_HTTP_PUT = 3,
	NK_HTTP_DELETE = 4,
	NK_HTTP_CONNECT = 5,
	NK_HTTP_OPTIONS = 6,
	NK_HTTP_TRACE = 7,
	NK_HTTP_PATCH = 8,
	NK_HTTP_UNDEFINED = 9
} nk_http_method_t;

NETKIT_API nk_status_t nk_http_headers_create(nk_http_headers_t** out);
NETKIT_API void nk_http_headers_destroy(nk_http_headers_t* headers);
NETKIT_API nk_status_t nk_http_headers_add(nk_http_headers_t* headers, const char* name, const char* value);
NETKIT_API size_t nk_http_headers_count(const nk_http_headers_t* headers);
NETKIT_API bool nk_http_headers_at(const nk_http_headers_t* headers, size_t index, char** out_name, char** out_value);
NETKIT_API bool nk_http_headers_find(const nk_http_headers_t* headers, const char* name, char** out_value);
NETKIT_API nk_status_t nk_http_client_create(const nk_addr_t* addr, nk_http_scheme_t scheme, nk_http_client_t** out);
NETKIT_API void nk_http_client_destroy(nk_http_client_t* client);
NETKIT_API void nk_http_client_set_user_agent(nk_http_client_t* client, const char* user_agent);
NETKIT_API void nk_http_client_set_accept(nk_http_client_t* client, const char* accept);
NETKIT_API void nk_http_client_set_content_type(nk_http_client_t* client, const char* content_type);
NETKIT_API void nk_http_client_set_close(nk_http_client_t* client, bool close);
NETKIT_API nk_status_t nk_http_client_request(nk_http_client_t* client, nk_http_method_t method, const char* path, nk_body_t* body, const nk_http_headers_t* headers, nk_http_response_t** out);
NETKIT_API nk_status_t nk_http_client_request_custom(nk_http_client_t* client, const char* method, const char* path, nk_body_t* body, const nk_http_headers_t* headers, nk_http_response_t** out);
NETKIT_API nk_status_t nk_http_client_get(nk_http_client_t* client, const char* path, const nk_http_headers_t* headers, nk_http_response_t** out);
NETKIT_API nk_status_t nk_http_client_post(nk_http_client_t* client, const char* path, nk_body_t* body, const nk_http_headers_t* headers, nk_http_response_t** out);
NETKIT_API nk_status_t nk_http_client_put(nk_http_client_t* client, const char* path, nk_body_t* body, const nk_http_headers_t* headers, nk_http_response_t** out);
NETKIT_API nk_status_t nk_http_client_patch(nk_http_client_t* client, const char* path, nk_body_t* body, const nk_http_headers_t* headers, nk_http_response_t** out);

NETKIT_API void nk_http_response_destroy(nk_http_response_t* response);
NETKIT_API int nk_http_response_status_code(const nk_http_response_t* response);
NETKIT_API const nk_http_headers_t* nk_http_response_headers(const nk_http_response_t* response);
NETKIT_API nk_body_t* nk_http_response_take_body(nk_http_response_t* response);

#ifdef __cplusplus
}
#endif
