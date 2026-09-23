/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file io.h
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief C bindings for netkit::io::io_context and netkit's cancellation.
 */
#pragma once

#include <netkit/c/common.h>
#include <netkit/c/types.h>

#ifdef __cplusplus
extern "C" {
#endif

NETKIT_API nk_status_t nk_io_context_create(nk_io_context_t** out);
NETKIT_API void nk_io_context_destroy(nk_io_context_t* ctx);
NETKIT_API nk_status_t nk_io_context_run(nk_io_context_t* ctx);
NETKIT_API nk_status_t nk_io_context_run_until_idle(nk_io_context_t* ctx);
NETKIT_API void nk_io_context_stop(nk_io_context_t* ctx);
NETKIT_API nk_status_t nk_cancellation_source_create(nk_cancellation_source_t** out);
NETKIT_API void nk_cancellation_source_destroy(nk_cancellation_source_t* source);
NETKIT_API void nk_cancellation_source_cancel(nk_cancellation_source_t* source);
NETKIT_API bool nk_cancellation_source_is_cancelled(const nk_cancellation_source_t* source);
NETKIT_API nk_status_t nk_cancellation_token_create(nk_cancellation_token_t** out);
NETKIT_API void nk_cancellation_token_destroy(nk_cancellation_token_t* token);
NETKIT_API void nk_cancellation_token_cancel(nk_cancellation_token_t* token);
NETKIT_API bool nk_cancellation_token_is_cancelled(const nk_cancellation_token_t* token);
NETKIT_API void nk_cancellation_token_reset(nk_cancellation_token_t* token);

#ifdef __cplusplus
}
#endif
