/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file netkit-crypto.cppm
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Module partition wrapping netkit's crypto:: headers (fallback CA data and, on Windows, cert-store helpers).
 */
module;

#include <netkit/definitions.hpp>
#include <netkit/crypto/fallback_ca.hpp>
#include <netkit/crypto/windows/certs.hpp>

export module netkit:crypto;

export namespace netkit::crypto {
    using namespace ::netkit::crypto;
}

#ifdef NETKIT_WINDOWS
export namespace netkit::crypto::windows {
    using namespace ::netkit::crypto::windows;
}
#endif