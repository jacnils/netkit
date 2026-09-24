/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file tls.h
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief C bindings for netkit::stream::tls_stream (netkit's WolfSSL-backed TLS stream).
 *  @note Only declared when netkit was built with WolfSSL support (NETKIT_WOLFSSL, set automatically
 *        by CMake's NETKIT_ENABLE_WOLFSSL option, which defaults to ON and propagates to consumers
 *        of the netkit CMake target). Guard your own code with `#ifdef NETKIT_WOLFSSL` if you need
 *        to support builds with WolfSSL disabled.
 *  @see netkit/stream/wolfssl/tls_stream.hpp
 */
#pragma once

#ifdef NETKIT_WOLFSSL

#include <netkit/c/common.h>
#include <netkit/c/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Mirrors netkit::stream::version. */
typedef enum nk_tls_version {
	NK_TLS_1_1 = 0,
	NK_TLS_1_2 = 1,
	NK_TLS_1_3 = 2
} nk_tls_version_t;

/** @brief Mirrors netkit::stream::verification. */
typedef enum nk_tls_verification {
	NK_TLS_VERIFY_PEER = 0,
	NK_TLS_VERIFY_NONE = 1
} nk_tls_verification_t;

/**
 * @brief Wraps an already-connected TCP stream in a (not-yet-handshaked) TLS session.
 * @param stream Ownership is transferred to the new TLS stream; do not use or destroy `stream`
 *        afterwards, whether this call succeeds or fails.
 * @param version Minimum TLS version to negotiate.
 * @param verification Whether to verify the peer's certificate.
 * @param ca_cert Optional PEM-encoded CA certificate to trust, in addition to netkit's built-in
 *        trust store; pass NULL or an empty string to use only the built-in trust store.
 * @param sni Optional server name to send via SNI; pass NULL or an empty string to omit it.
 * @param out Receives the new TLS stream on success. Must not be NULL.
 * @note Call nk_tls_stream_handshake() before reading or writing.
 */
NETKIT_API nk_status_t nk_tls_stream_create(nk_tcp_stream_t* stream, nk_tls_version_t version,
                                             nk_tls_verification_t verification, const char* ca_cert,
                                             const char* sni, nk_tls_stream_t** out);

/** @brief Destroys a TLS stream, closing it first if still open. Safe with NULL. */
NETKIT_API void nk_tls_stream_destroy(nk_tls_stream_t* stream);

/** @brief Performs the TLS handshake. Must be called once, after connect and before any I/O. */
NETKIT_API nk_status_t nk_tls_stream_handshake(nk_tls_stream_t* stream);

/** @brief Closes the TLS session (and the underlying TCP connection). Safe to call multiple times. */
NETKIT_API void nk_tls_stream_close(nk_tls_stream_t* stream);
/** @brief True if the TLS session is open. */
NETKIT_API bool nk_tls_stream_is_open(const nk_tls_stream_t* stream);

/**
 * @brief Returns a generic stream view over this TLS stream, for use with stream.h and body.h.
 * @note The returned view is non-owning and stays valid only as long as `stream` is alive.
 */
NETKIT_API nk_stream_t* nk_tls_stream_as_stream(nk_tls_stream_t* stream);

#ifdef __cplusplus
}
#endif

#endif /* NETKIT_WOLFSSL */
