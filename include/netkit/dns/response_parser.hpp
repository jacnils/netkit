/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file response_parser.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides a DNS response parser class to parse DNS response packets.
 */
#pragma once

#include <netkit/export.hpp>
#include <netkit/dns/cache.hpp>
#include <netkit/dns/record_type.hpp>

namespace netkit::dns {
    class NETKIT_API response_parser {
        const std::vector<uint8_t>& data;
        size_t offset = 0;

        uint16_t read_uint16();
        uint32_t read_uint32();
        std::string decode_name(size_t pos_override = std::string::npos);
    public:
        explicit response_parser(const std::vector<uint8_t>& bytes);
        std::vector<dns::record> parse();
    };
}