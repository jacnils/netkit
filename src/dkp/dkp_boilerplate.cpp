/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file dkp_boilerplate.cpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Jank that calls net_init()
 */
#include <netkit/except.hpp>
#include <netkit/definitions.hpp>

#if NETKIT_DKP
#include <network.h>
#include <stdexcept>

namespace netkit::internal_net {
    inline void ensure_net_initialized() {
        static bool initialized = [] {
			net_init();

            return true;
        }();
        static_cast<void>(initialized);
    }

    struct network_auto_init {
        network_auto_init() {
            netkit::internal_net::ensure_net_initialized();
        }
    };

    [[maybe_unused]] static network_auto_init _network_init;
}
#endif