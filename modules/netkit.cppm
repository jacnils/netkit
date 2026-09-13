/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file netkit.cppm
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Primary C++23 module interface for netkit.
 */
export module netkit;

export import :except;
export import :utility;
export import :network;
export import :socket;
export import :io;
export import :stream;
export import :body;
export import :tcp;
export import :datagram;
export import :udp;
export import :uds;
export import :dns;
export import :http;
export import :crypto;
