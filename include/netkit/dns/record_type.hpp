/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file record_type.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides a set of DNS record types and associated data structures.
 */
#pragma once

#include <vector>
#include <string>
#include <variant>
#include <chrono>

#include <netkit/sock/ip_list.hpp>

namespace netkit::dns {
    using ip_list = netkit::sock::ip_list;

    enum class record_type {
        A = 1,
        AAAA = 28,
        CNAME = 5,
        MX = 15,
        NS = 2,
        TXT = 16,
        SOA = 6,
        SRV = 33,
        PTR = 12,
        CAA = 257,
        ANY = 255,
        OTHER = 0,
    };

    struct generic_record_data {
        uint16_t type{};
        std::vector<uint8_t> raw{};
    };

    struct a_record_data {
        ip_list ip{};
    };

    struct aaaa_record_data {
        ip_list ip{};
    };

    struct cname_record_data {
        std::string cname{};
    };

    struct mx_record_data {
        uint16_t preference{};
        std::string exchange{};
    };

    struct ns_record_data {
        std::string ns{};
    };

    struct txt_record_data {
        std::vector<std::string> text{};
    };

    struct soa_record_data {
        std::string mname{};
        std::string rname{};
        uint32_t serial{}, refresh{}, retry{}, expire{}, minimum{};
    };

    struct srv_record_data {
        uint16_t priority{}, weight{}, port{};
        std::string target{};
    };

    struct ptr_record_data {
        std::string ptrname{};
    };

    struct caa_record_data {
        uint8_t flags{};
        std::string tag{};
        std::string value{};
    };

    using record_data = std::variant<
        a_record_data,
        aaaa_record_data,
        cname_record_data,
        mx_record_data,
        ns_record_data,
        txt_record_data,
        soa_record_data,
        srv_record_data,
        ptr_record_data,
        caa_record_data,
        generic_record_data,
        std::monostate
    >;


    struct record {
        std::string name{};
        record_type type{};
        uint16_t record_class{1};
        uint32_t ttl{};
        record_data data{};
        int64_t created_at{std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count()};

        void serialize(std::ostream& os) const;
        static record deserialize(std::istream& is);
    };

    inline bool operator==(const a_record_data& lhs, const a_record_data& rhs) {
        return lhs.ip.get_ipv4() == rhs.ip.get_ipv4() && lhs.ip.get_ipv6() == rhs.ip.get_ipv6();
    }

    inline bool operator==(const aaaa_record_data& lhs, const aaaa_record_data& rhs) {
        return lhs.ip.get_ipv4() == rhs.ip.get_ipv4() && lhs.ip.get_ipv6() == rhs.ip.get_ipv6();
    }

    inline bool operator==(const cname_record_data& lhs, const cname_record_data& rhs) {
        return lhs.cname == rhs.cname;
    }

    inline bool operator==(const mx_record_data& lhs, const mx_record_data& rhs) {
        return lhs.preference == rhs.preference && lhs.exchange == rhs.exchange;
    }

    inline bool operator==(const ns_record_data& lhs, const ns_record_data& rhs) {
        return lhs.ns == rhs.ns;
    }

    inline bool operator==(const txt_record_data& lhs, const txt_record_data& rhs) {
        return lhs.text == rhs.text;
    }

    inline bool operator==(const soa_record_data& lhs, const soa_record_data& rhs) {
        return lhs.mname == rhs.mname &&
               lhs.rname == rhs.rname &&
               lhs.serial == rhs.serial &&
               lhs.refresh == rhs.refresh &&
               lhs.retry == rhs.retry &&
               lhs.expire == rhs.expire &&
               lhs.minimum == rhs.minimum;
    }

    inline bool operator==(const srv_record_data& lhs, const srv_record_data& rhs) {
        return lhs.priority == rhs.priority &&
               lhs.weight == rhs.weight &&
               lhs.port == rhs.port &&
               lhs.target == rhs.target;
    }

    inline bool operator==(const ptr_record_data& lhs, const ptr_record_data& rhs) {
        return lhs.ptrname == rhs.ptrname;
    }

    inline bool operator==(const caa_record_data& lhs, const caa_record_data& rhs) {
        return lhs.flags == rhs.flags &&
               lhs.tag == rhs.tag &&
               lhs.value == rhs.value;
    }

    inline bool operator==(const generic_record_data& lhs, const generic_record_data& rhs) {
        return lhs.type == rhs.type && lhs.raw == rhs.raw;
    }
}