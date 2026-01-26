/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file response_parser.cpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides an implementation of the DNS response parser class.
 */
#include <cstdint>

#include <netkit/except.hpp>
#include <netkit/definitions.hpp>
#include <netkit/dns/response_parser.hpp>

#ifdef NETKIT_UNIX
#include <arpa/inet.h>
#include <netinet/in.h>
#endif
#ifdef NETKIT_WINDOWS
#include <ws2tcpip.h>
#endif

uint16_t netkit::dns::response_parser::read_uint16() {
    uint16_t val = (data[offset] << 8) | data[offset + 1];
    offset += 2;
    return val;
}

uint32_t netkit::dns::response_parser::read_uint32() {
    uint32_t val = (static_cast<uint32_t>(data[offset]) << 24) |
                   (static_cast<uint32_t>(data[offset + 1]) << 16) |
                   (static_cast<uint32_t>(data[offset + 2]) << 8) |
                   (static_cast<uint32_t>(data[offset + 3]));
    offset += 4;
    return val;
}

std::string netkit::dns::response_parser::decode_name(size_t pos_override) {
    std::string name;
    size_t pos = (pos_override == std::string::npos) ? offset : pos_override;
    bool jumped = false;
    size_t jump_offset = 0;

    while (true) {
        uint8_t len = data[pos];
        if ((len & 0xC0) == 0xC0) {
            uint16_t pointer = ((len & 0x3F) << 8) | data[pos + 1];
            if (!jumped) jump_offset = pos + 2;
            pos = pointer;
            jumped = true;
            continue;
        }

        if (len == 0) {
            if (!jumped)
                offset = pos + 1;
            else
                offset = jump_offset;
            break;
        }

        if (!name.empty()) name += ".";
        name += std::string(reinterpret_cast<const char*>(&data[pos + 1]), len);
        pos += len + 1;
    }

    return name;
}

netkit::dns::response_parser::response_parser(const std::vector<uint8_t>& bytes) : data(bytes) {
    if (data.size() < 12) throw netkit::parsing_error("DNS response too short");
}

std::vector<netkit::dns::record> netkit::dns::response_parser::parse() {
    offset = 0;

    read_uint16();
    read_uint16();
    uint16_t qdcount = read_uint16();
    uint16_t ancount = read_uint16();
    read_uint16();
    read_uint16();

    for (int i = 0; i < qdcount; ++i) {
        decode_name();
        read_uint16();
        read_uint16();
    }

    std::vector<dns::record> results;

    for (int i = 0; i < ancount; ++i) {
        std::string name = decode_name();
        uint16_t type = read_uint16();
        uint16_t record_class = read_uint16();
        uint32_t ttl = read_uint32();
        uint16_t rdlength = read_uint16();

        if (type == 0) {
            throw parsing_error("DNS response contains a zero type record");
        }

        if (offset + rdlength > data.size()) {
            throw parsing_error("rdata length out of bounds");
        }

        dns::record rec;
        rec.name = std::move(name);
        rec.record_class = record_class;
        rec.ttl = ttl;

        if (type == 1 && rdlength == 4) { // A
            char ipbuf[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &data[offset], ipbuf, sizeof(ipbuf));
            rec.type = dns::record_type::A;
            rec.data = dns::a_record_data{{std::string(ipbuf), ""}};
            offset += rdlength;
        } else if (type == 28 && rdlength == 16) { // AAAA
            char ipbuf[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, &data[offset], ipbuf, sizeof(ipbuf));
            rec.type = dns::record_type::AAAA;
            rec.data = dns::aaaa_record_data{{"", std::string(ipbuf)}};
            offset += rdlength;
        } else if (type == 5) { // CNAME
            auto saved_offset = offset;
            std::string cname_target = decode_name(offset);
            rec.type = dns::record_type::CNAME;
            rec.data = dns::cname_record_data{std::move(cname_target)};
            offset = saved_offset + rdlength;
        } else if (type == 15) {
            // MX
            if (rdlength < 3) throw parsing_error("malformed MX record");

            uint16_t preference = (data[offset] << 8) | data[offset + 1];
            auto saved_offset = offset;
            std::string exchange = decode_name(offset + 2);
            rec.type = dns::record_type::MX;
            rec.data = dns::mx_record_data{preference, std::move(exchange)};
            offset = saved_offset + rdlength;
        } else if (type == 2) { // NS
            auto saved_offset = offset;
            std::string nsdname = decode_name(offset);
            rec.type = dns::record_type::NS;
            rec.data = dns::ns_record_data{std::move(nsdname)};
            offset = saved_offset + rdlength;
        } else if (type == 16) { // TXT
            std::vector<std::string> texts;
            size_t end = offset + rdlength;
            while (offset < end) {
                uint8_t txt_len = data[offset++];
                if (offset + txt_len > end) {
                    throw parsing_error("TXT record string length out of bounds");
                }
                texts.emplace_back(reinterpret_cast<const char*>(&data[offset]), txt_len);
                offset += txt_len;
            }
            rec.type = dns::record_type::TXT;
            rec.data = dns::txt_record_data{std::move(texts)};
        } else if (type == 33) { // SRV
            if (rdlength < 6) {
                throw parsing_error("SRV record too short");
            }

            uint16_t priority = read_uint16();
            uint16_t weight = read_uint16();
            uint16_t port = read_uint16();

            std::string target = decode_name();

            rec.type = dns::record_type::SRV;
            rec.data = dns::srv_record_data{priority, weight, port, std::move(target)};
        } else if (type == 12) { // PTR
            std::string ptr_name = decode_name();
            rec.type = dns::record_type::PTR;
            rec.data = dns::ptr_record_data{std::move(ptr_name)};
        } else if (type == 6) { // SOA
            std::string mname = decode_name();
            std::string rname = decode_name();

            uint32_t serial = read_uint32();
            uint32_t refresh = read_uint32();
            uint32_t retry = read_uint32();
            uint32_t expire = read_uint32();
            uint32_t minimum = read_uint32();

            rec.type = dns::record_type::SOA;
            rec.data = dns::soa_record_data{
                std::move(mname),
                std::move(rname),
                serial,
                refresh,
                retry,
                expire,
                minimum
            };
        } else {
            rec.type = dns::record_type::OTHER;
            rec.data = dns::generic_record_data{
                type, std::vector<uint8_t>{data.begin() + static_cast<long>(offset), data.begin() + static_cast<long>(offset) + rdlength}
            };
            offset += rdlength;
        }

        results.push_back(std::move(rec));
    }

    return results;
}
