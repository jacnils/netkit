/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file netkit-dns.cppm
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Module partition wrapping netkit's dns:: headers.
 */
module;

#include <netkit/definitions.hpp>
#include <netkit/dns/record_type.hpp>
#include <netkit/dns/sync_resolver.hpp>
#include <netkit/dns/nameserver_list.hpp>
#include <netkit/dns/cache.hpp>
#include <netkit/dns/response_parser.hpp>
#include <netkit/dns/query_builder.hpp>

export module netkit:dns;

export namespace netkit::dns {
    using namespace ::netkit::dns;
}