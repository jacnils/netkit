/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file stream.h
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief C bindings for netkit::stream::basic_stream. Use casting.
 */
#pragma once

#include <netkit/c/common.h>
#include <netkit/c/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum nk_stream_status {
	NK_STREAM_SUCCESS = 0,
	NK_STREAM_CLOSED = 1,
	NK_STREAM_ERROR = 2,
	NK_STREAM_EOF = 3
} nk_stream_status_t;

typedef struct nk_stream_result {
	size_t bytes;
	nk_stream_status_t status;
} nk_stream_result_t;

NETKIT_API nk_stream_result_t nk_stream_read(nk_stream_t* stream, void* buffer, size_t len);
NETKIT_API nk_stream_result_t nk_stream_write(nk_stream_t* stream, const void* buffer, size_t len);
NETKIT_API nk_stream_result_t nk_stream_write_all(nk_stream_t* stream, const void* buffer, size_t len);
NETKIT_API nk_stream_result_t nk_stream_write_all_body(nk_stream_t* stream, nk_body_t* body);
NETKIT_API void nk_stream_close(nk_stream_t* stream);
NETKIT_API bool nk_stream_is_open(const nk_stream_t* stream);
NETKIT_API nk_status_t nk_stream_read_all(nk_stream_t* stream, size_t max_bytes, char** out_data, size_t* out_len);
NETKIT_API nk_status_t nk_stream_get_addr(nk_stream_t* stream, nk_addr_t** out);

#ifdef __cplusplus
}
#endif
