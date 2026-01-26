/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file query_builder.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides a class for building DNS query packets.
 */
#pragma once

#include <netkit/dns/cache.hpp>
#include <netkit/dns/record_type.hpp>

namespace netkit::dns {
    class query_builder {
        std::vector<uint8_t> packet;
        uint16_t id;
        bool recursion{true};

        void encode_name(const std::string& name);
        void write_uint16_t(uint16_t value);
        void write_uint32_t(uint32_t value);
    public:
        explicit query_builder(uint16_t _id = 0);
        void set_recursion_desired(bool desired);
        void add_question(const std::string& name, dns::record_type type = dns::record_type::A, uint16_t record_class = 1);
        const std::vector<uint8_t>& build();
    };
}