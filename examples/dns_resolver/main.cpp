/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file main.cpp
 *  @license MIT
 *  @note Example code using the Netkit library.
 */
#include <iostream>
#include <string>
#include <variant>

#include <netkit/dns/record_type.hpp>
#include <netkit/dns/sync_resolver.hpp>

using namespace netkit::dns;

const char* record_type_name(record_type type) {
    switch (type) {
    case record_type::A:     return "A";
    case record_type::AAAA:  return "AAAA";
    case record_type::CNAME: return "CNAME";
    case record_type::MX:    return "MX";
    case record_type::NS:    return "NS";
    case record_type::TXT:   return "TXT";
    case record_type::SOA:   return "SOA";
    case record_type::SRV:   return "SRV";
    case record_type::PTR:   return "PTR";
    case record_type::CAA:   return "CAA";
    case record_type::ANY:   return "ANY";
    case record_type::OTHER: return "OTHER";
    }

    return "UNKNOWN";
}

void print_record_data(const record_data& data) {
    std::visit([]<typename T0>(const T0& value) {
        using T = std::decay_t<T0>;

        if constexpr (std::is_same_v<T, a_record_data>) {
            for (const auto& ip : value.ip.get_ipv4())
                std::cout << "    " << ip << '\n';

        } else if constexpr (std::is_same_v<T, aaaa_record_data>) {
            for (const auto& ip : value.ip.get_ipv6())
                std::cout << "    " << ip << '\n';

        } else if constexpr (std::is_same_v<T, cname_record_data>) {
            std::cout << "    " << value.cname << '\n';

        } else if constexpr (std::is_same_v<T, mx_record_data>) {
            std::cout << "    preference: " << value.preference << '\n';
            std::cout << "    exchange:   " << value.exchange << '\n';

        } else if constexpr (std::is_same_v<T, ns_record_data>) {
            std::cout << "    " << value.ns << '\n';

        } else if constexpr (std::is_same_v<T, txt_record_data>) {
            for (const auto& text : value.text)
                std::cout << "    \"" << text << "\"\n";

        } else if constexpr (std::is_same_v<T, soa_record_data>) {
            std::cout << "    mname:   " << value.mname << '\n';
            std::cout << "    rname:   " << value.rname << '\n';
            std::cout << "    serial:  " << value.serial << '\n';
            std::cout << "    refresh: " << value.refresh << '\n';
            std::cout << "    retry:   " << value.retry << '\n';
            std::cout << "    expire:  " << value.expire << '\n';
            std::cout << "    minimum: " << value.minimum << '\n';

        } else if constexpr (std::is_same_v<T, srv_record_data>) {
            std::cout << "    priority: " << value.priority << '\n';
            std::cout << "    weight:   " << value.weight << '\n';
            std::cout << "    port:     " << value.port << '\n';
            std::cout << "    target:   " << value.target << '\n';

        } else if constexpr (std::is_same_v<T, ptr_record_data>) {
            std::cout << "    " << value.ptrname << '\n';

        } else if constexpr (std::is_same_v<T, caa_record_data>) {
            std::cout << "    flags: " << static_cast<unsigned>(value.flags) << '\n';
            std::cout << "    tag:   " << value.tag << '\n';
            std::cout << "    value: " << value.value << '\n';

        } else if constexpr (std::is_same_v<T, generic_record_data>) {
            std::cout << "    unknown type: " << value.type << '\n';
            std::cout << "    raw data: " << value.raw.size() << " bytes\n";

        } else if constexpr (std::is_same_v<T, std::monostate>) {
            std::cout << "    <no data>\n";
        }
    }, data);
}

void print_record(const record& r) {
    std::cout << "  " << r.name
              << "  TTL=" << r.ttl
              << "  CLASS=" << r.record_class
              << '\n';

    print_record_data(r.data);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: dns_example <hostname>\n";
        return 1;
    }

    const std::string hostname = argv[1];

    constexpr record_type types[] = {
        record_type::A,
        record_type::AAAA,
        record_type::CNAME,
        record_type::MX,
        record_type::NS,
        record_type::TXT,
        record_type::SOA,
        record_type::SRV,
        record_type::PTR,
        record_type::CAA,
    };

    try {
		std::cout << "DNS records for " << hostname << "\n\n";

        for (const auto type : types) {
            std::cout << '[' << record_type_name(type) << "]\n";

            try {
				sync_resolver resolver; // dns over tls supported, pass in true to the 2nd parameter
				const auto records = resolver.query_records(hostname, type);

                for (const auto& record : records)
                    print_record(record);

            } catch (const netkit::dns_error&) {
                std::cout << "  <no records>\n";
            }

            std::cout << '\n';
        }
    } catch (const std::exception& e) {
        std::cerr << "DNS lookup failed: " << e.what() << '\n';
        return 1;
    }

    return 0;
}