/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file common.h
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Status enums and helpers shared by netkit's C bindings.
 */
#pragma once

#include <netkit/export.hpp>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum nk_status {
	NK_OK = 0,
	NK_ERR_GENERIC = 1,           /* netkit::generic_error */
	NK_ERR_SOCKET = 2,            /* netkit::socket_error */
	NK_ERR_PARSING = 3,           /* netkit::parsing_error */
	NK_ERR_IP = 4,                /* netkit::ip_error */
	NK_ERR_DNS = 5,               /* netkit::dns_error */
	NK_ERR_SSL = 6,               /* netkit::ssl_error */
	NK_ERR_LOGIC = 7,             /* netkit::logic_error */
	NK_ERR_LENGTH = 8,            /* netkit::length_error */
	NK_ERR_IO = 9,                /* netkit::io_error */
	NK_ERR_INVALID_ARGUMENT = 10, /* bad argument */
	NK_ERR_UNKNOWN = 99           /* any other std::exception, or an unknown exception */
} nk_status_t;

NETKIT_API const char* nk_last_error(void);
NETKIT_API void nk_free_string(char* str);

#ifdef __cplusplus
}
#endif
