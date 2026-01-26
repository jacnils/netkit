/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file windows_boilerplate.cpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Quick Windows-only boilerplate to ensure Winsock is initialized.
 */
#include <netkit/except.hpp>
#include <netkit/definitions.hpp>

#if NETKIT_WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdexcept>

/* This is all a cheap bandaid fix.
 * I started writing all the Linux code initially, and then when I ported the socket code to Windows, it turns out WSAStartup() needs to be called before
 * use, and WSACleanup() needs to be called afterwards.
 * As a quick hacky fix, I came up with this design pattern. Ideally, we should move this to the socket class, but this will have to do as of now.
 * TODO: Move to the socket classes.
 */
namespace netkit::internal_net {
    inline void ensure_winsock_initialized() {
        static bool initialized = [] {
            WSADATA wsa_data;
            int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
            if (result != 0) {
                throw std::runtime_error("WSAStartup failed with code " + std::to_string(result));
            }

            std::atexit([] {
                WSACleanup();
            });

            return true;
        }();
        static_cast<void>(initialized);
    }

    struct winsock_auto_init {
        winsock_auto_init() {
            netkit::internal_net::ensure_winsock_initialized();
        }
    };

    [[maybe_unused]] static winsock_auto_init _winsock_init;
}
#endif