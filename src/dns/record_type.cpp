/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file record_type.cpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Implementation of the DNS record type serialization and deserialization.
 */
#include <netkit/dns/record_type.hpp>
#include <netkit/utility.hpp>

void netkit::dns::record::serialize(std::ostream& os) const {
    utility::write_string(os, name);
    utility::write<uint16_t>(os, static_cast<uint16_t>(type));
    utility::write<uint16_t>(os, record_class);
    utility::write<uint32_t>(os, ttl);
    utility::write<int64_t>(os, created_at);

    std::visit([&]<typename T0>(T0&& val) {
        using T = std::decay_t<T0>;
        if constexpr (std::is_same_v<T, a_record_data>) {
            utility::write<uint8_t>(os, 1);
            utility::write_string(os, val.ip.get_ip());
        } else if constexpr (std::is_same_v<T, aaaa_record_data>) {
            utility::write<uint8_t>(os, 2);
            utility::write_string(os, val.ip.get_ip());
        } else if constexpr (std::is_same_v<T, cname_record_data>) {
            utility::write<uint8_t>(os, 3);
            utility::write_string(os, val.cname);
        } else if constexpr (std::is_same_v<T, mx_record_data>) {
            utility::write<uint8_t>(os, 4);
            utility::write<uint16_t>(os, val.preference);
            utility::write_string(os, val.exchange);
        } else if constexpr (std::is_same_v<T, ns_record_data>) {
            utility::write<uint8_t>(os, 5);
            utility::write_string(os, val.ns);
        } else if constexpr (std::is_same_v<T, txt_record_data>) {
            utility::write<uint8_t>(os, 6);
            uint32_t count = val.text.size();
            utility::write(os, count);
            for (const auto& s : val.text) {
                utility::write_string(os, s);
            }
        } else if constexpr (std::is_same_v<T, soa_record_data>) {
            utility::write<uint8_t>(os, 7);
            utility::write_string(os, val.mname);
            utility::write_string(os, val.rname);
            utility::write(os, val.serial);
            utility::write(os, val.refresh);
            utility::write(os, val.retry);
            utility::write(os, val.expire);
            utility::write(os, val.minimum);
        } else if constexpr (std::is_same_v<T, srv_record_data>) {
            utility::write<uint8_t>(os, 8);
            utility::write(os, val.priority);
            utility::write(os, val.weight);
            utility::write(os, val.port);
            utility::write_string(os, val.target);
        } else if constexpr (std::is_same_v<T, ptr_record_data>) {
            utility::write<uint8_t>(os, 9);
            utility::write_string(os, val.ptrname);
        } else if constexpr (std::is_same_v<T, caa_record_data>) {
            utility::write<uint8_t>(os, 10);
            utility::write(os, val.flags);
            utility::write_string(os, val.tag);
            utility::write_string(os, val.value);
        } else if constexpr (std::is_same_v<T, generic_record_data>) {
            utility::write<uint8_t>(os, 11);
            utility::write(os, val.type);
            utility::write<uint32_t>(os, val.raw.size());
            os.write(reinterpret_cast<const char*>(val.raw.data()), val.raw.size());
        } else if constexpr (std::is_same_v<T, std::monostate>) {
            utility::write<uint8_t>(os, 0);
        }
    }, data);
}

netkit::dns::record netkit::dns::record::deserialize(std::istream& is) {
    record rec;
    rec.name = utility::read_string(is);

    uint16_t type_raw;
    utility::read(is, type_raw);
    rec.type = static_cast<record_type>(type_raw);

    utility::read(is, rec.record_class);
    utility::read(is, rec.ttl);
    utility::read<int64_t>(is, rec.created_at);

    uint8_t tag;
    utility::read(is, tag);

    switch (tag) {
    case 1: {
            a_record_data a;
            a.ip = ip_list(utility::read_string(is));
            rec.data = a;
            break;
    }
    case 2: {
            aaaa_record_data aaaa;
            aaaa.ip = ip_list(utility::read_string(is));
            rec.data = aaaa;
            break;
    }
    case 3: {
            cname_record_data c;
            c.cname = utility::read_string(is);
            rec.data = c;
            break;
    }
    case 4: {
            mx_record_data m;
            utility::read(is, m.preference);
            m.exchange = utility::read_string(is);
            rec.data = m;
            break;
    }
    case 5: {
            ns_record_data n;
            n.ns = utility::read_string(is);
            rec.data = n;
            break;
    }
    case 6: {
            txt_record_data t;
            uint32_t count;
            utility::read(is, count);
            t.text.resize(count);
            for (auto& s : t.text) {
                s = utility::read_string(is);
            }
            rec.data = t;
            break;
    }
    case 7: {
            soa_record_data soa;
            soa.mname = utility::read_string(is);
            soa.rname = utility::read_string(is);
            utility::read(is, soa.serial);
            utility::read(is, soa.refresh);
            utility::read(is, soa.retry);
            utility::read(is, soa.expire);
            utility::read(is, soa.minimum);
            rec.data = soa;
            break;
    }
    case 8: {
            srv_record_data srv;
            utility::read(is, srv.priority);
            utility::read(is, srv.weight);
            utility::read(is, srv.port);
            srv.target = utility::read_string(is);
            rec.data = srv;
            break;
    }
    case 9: {
            ptr_record_data p;
            p.ptrname = utility::read_string(is);
            rec.data = p;
            break;
    }
    case 10: {
            caa_record_data c;
            utility::read(is, c.flags);
            c.tag = utility::read_string(is);
            c.value = utility::read_string(is);
            rec.data = c;
            break;
    }
    case 11: {
            generic_record_data g;
            utility::read(is, g.type);
            uint32_t len;
            utility::read(is, len);
            g.raw.resize(len);
            is.read(reinterpret_cast<char*>(g.raw.data()), len);
            rec.data = g;
            break;
    }
    case 0:
    default:
        rec.data = std::monostate{};
    }

    return rec;
}

