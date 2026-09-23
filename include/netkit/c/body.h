/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file body.h
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief C bindings for netkit::body::basic_body and its buffer/file/stream/chunked implementations.
 */
#pragma once

#include <netkit/c/common.h>
#include <netkit/c/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum nk_read_status {
	NK_READ_OK = 0,
	NK_READ_EOF = 1,
	NK_READ_ERROR = 2,
	NK_READ_TIMEOUT = 3
} nk_read_status_t;

typedef struct nk_read_result {
	nk_read_status_t status;
	size_t bytes_read;
} nk_read_result_t;

NETKIT_API nk_status_t nk_body_create_buffer(nk_body_t** out);
NETKIT_API nk_status_t nk_body_create_buffer_from(const char* data, size_t len, nk_body_t** out);

NETKIT_API nk_status_t nk_body_create_file(const char* path, nk_body_t** out);
NETKIT_API bool nk_body_file_is_open(const nk_body_t* body);

NETKIT_API nk_status_t nk_body_create_stream(nk_stream_t* stream, bool has_length, size_t length,
                                              const char* initial, size_t initial_len, nk_body_t** out);

NETKIT_API nk_status_t nk_body_create_chunked(nk_stream_t* stream, const char* initial, size_t initial_len,
                                               nk_body_t** out);

NETKIT_API void nk_body_destroy(nk_body_t* body);

NETKIT_API nk_read_result_t nk_body_read(nk_body_t* body, char* buffer, size_t max_bytes);

NETKIT_API nk_status_t nk_body_read_all(nk_body_t* body, bool has_max_size, size_t max_size, char** out_data,
                                         size_t* out_len);

NETKIT_API bool nk_body_size(const nk_body_t* body, size_t* out_size);
NETKIT_API bool nk_body_empty(const nk_body_t* body);
NETKIT_API bool nk_body_rewind(nk_body_t* body);

NETKIT_API nk_status_t nk_body_buffer_set(nk_body_t* body, const char* data, size_t len);
NETKIT_API nk_status_t nk_body_buffer_append(nk_body_t* body, const char* data, size_t len);

#ifdef __cplusplus
}
#endif
