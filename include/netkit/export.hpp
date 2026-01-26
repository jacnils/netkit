/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file export.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @note Useless to end users; used internally by the build system.
 *  @note Windows specific; does not affect other platforms.
 *  @brief Provides export definitions for building and using the Netkit library as a shared library.
 */
#pragma once

#if defined(_WIN32)
  #if defined(NETKIT_BUILD_DLL)
    #define NETKIT_API __declspec(dllexport)
  #else
    #define NETKIT_API __declspec(dllimport)
  #endif
#else
  #define NETKIT_API __attribute__((visibility("default")))
#endif
