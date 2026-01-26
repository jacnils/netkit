/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file definitions.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides platform detection and common definitions used throughout the Netkit library.
 */
#pragma once

#if defined(__APPLE__)
#define NETKIT_MACOS 1
#endif
#if defined(__unix__) || defined(__unix) || defined(__APPLE__) && defined(__MACH__)
#define NETKIT_UNIX 1
#endif
#ifdef _WIN32
#define NETKIT_WINDOWS 1
#endif

#ifndef NETKIT_FALLBACK_IPV4_DNS_1
#define NETKIT_FALLBACK_IPV4_DNS_1 "8.8.8.8"
#endif
#ifndef NETKIT_FALLBACK_IPV4_DNS_2
#define NETKIT_FALLBACK_IPV4_DNS_2 "8.8.4.4"
#endif
#ifndef NETKIT_FALLBACK_IPV6_DNS_1
#define NETKIT_FALLBACK_IPV6_DNS_1 "2001:4860:4860::8888"
#endif
#ifndef NETKIT_FALLBACK_IPV6_DNS_2
#define NETKIT_FALLBACK_IPV6_DNS_2 "2001:4860:4860::8844"
#endif
#ifndef NETKIT_LOCALHOST_IPV4
#define NETKIT_LOCALHOST_IPV4 "127.0.0.1"
#endif
#ifndef NETKIT_LOCALHOST_IPV6
#define NETKIT_LOCALHOST_IPV6 "::1"
#endif

/* Not ever to be used by any external code */
static constexpr int ns_t_a{1};
static constexpr int ns_t_ns{2};
static constexpr int ns_t_cname{5};
static constexpr int ns_t_soa{6};
static constexpr int ns_t_ptr{12};
static constexpr int ns_t_mx{15};
static constexpr int ns_t_txt{16};
static constexpr int ns_t_aaaa{28};
static constexpr int ns_t_srv{33};
static constexpr int ns_t_any{255};
static constexpr int ns_t_caa{257};