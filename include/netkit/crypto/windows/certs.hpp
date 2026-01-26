/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file certs.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @note Windows specific; requires OpenSSL.
 *  @note External use of this header is discouraged and its API may change at any time without notice.
 *  @note Used internally by Netkit for certificate management on Windows, since OpenSSL does not natively interface with the Windows certificate store.
 *  @brief Provides functions to check for outdated certificates and export certificates on Windows using OpenSSL.
 */
#pragma once

#include <netkit/definitions.hpp>

#ifdef NETKIT_WINDOWS
#ifdef NETKIT_OPENSSL

#include <string>
#include <openssl/types.h>

namespace netkit::crypto::windows {
    bool is_outdated(const std::wstring& path);
    bool export_certs(const std::wstring& path);
}

#endif
#endif