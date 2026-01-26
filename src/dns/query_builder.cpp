/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file query_builder.cpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Implementation of the DNS query builder class.
 */
#include <cstdint>
#include <random>
#include <netkit/dns/query_builder.hpp>
#ifdef NETKIT_UNIX
#include <arpa/inet.h>
#endif

void netkit::dns::query_builder::encode_name(const std::string& name) {
    size_t start = 0;
    while (true) {
        size_t pos = name.find('.', start);
        if (pos == std::string::npos) pos = name.size();
        size_t len = pos - start;
        packet.push_back(static_cast<uint8_t>(len));
        for (size_t i = 0; i < len; ++i) {
            packet.push_back(name[start + i]);
        }
        if (pos == name.size()) break;
        start = pos + 1;
    }
    packet.push_back(0);
}
void netkit::dns::query_builder::write_uint16_t(uint16_t value) {
    packet.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    packet.push_back(static_cast<uint8_t>(value & 0xFF));
}
void netkit::dns::query_builder::write_uint32_t(uint32_t value) {
    packet.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
    packet.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
}

netkit::dns::query_builder::query_builder(uint16_t _id): id(_id) {
    if (id == 0) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<uint16_t> dis(1, 0xFFFF);
        this->id = dis(gen);
    }

    packet.resize(12);

    packet[0] = static_cast<uint8_t>((id >> 8) & 0xFF);
    packet[1] = static_cast<uint8_t>(id & 0xFF);

    uint16_t flags = recursion ? 0x0100 : 0x0000;
    packet[2] = static_cast<uint8_t>((flags >> 8) & 0xFF);
    packet[3] = static_cast<uint8_t>(flags & 0xFF);

    for (int i = 4; i < 12; ++i) {
        packet[i] = 0;
    }
}

void netkit::dns::query_builder::set_recursion_desired(bool desired) {
    recursion = desired;
    uint16_t flags = recursion ? 0x0100 : 0x0000;
    packet[2] = static_cast<uint8_t>((flags >> 8) & 0xFF);
    packet[3] = static_cast<uint8_t>(flags & 0xFF);
}
void netkit::dns::query_builder::add_question(const std::string& name, dns::record_type type, uint16_t record_class) {
    encode_name(name);
    write_uint16_t(static_cast<uint16_t>(type));
    write_uint16_t(record_class);

    uint16_t qdcount = (packet[4] << 8) | packet[5];
    ++qdcount;

    packet[4] = static_cast<uint8_t>((qdcount >> 8) & 0xFF);
    packet[5] = static_cast<uint8_t>(qdcount & 0xFF);
}
const std::vector<uint8_t>& netkit::dns::query_builder::build() {
    return this->packet;
}
