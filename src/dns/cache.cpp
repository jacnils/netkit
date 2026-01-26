/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file cache.cpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Implementation of the standard DNS cache class.
 */
#include <fstream>
#include <algorithm>

#include <netkit/dns/cache.hpp>
#include <netkit/dns/record_type.hpp>
#include <netkit/utility.hpp>

[[nodiscard]] std::vector<netkit::dns::record> netkit::dns::standard_cache::lookup(const std::string& hostname) const {
    std::ifstream is(utility::get_standard_cache_location(), std::ios::binary);
    if (!is) {
        return {};
    }

    while (is.peek() != EOF) {
        std::string name = utility::read_string(is);

        uint32_t count = 0;
        utility::read(is, count);

        std::vector<dns::record> records;
        records.reserve(count);
        for (uint32_t i = 0; i < count; ++i) {
            records.push_back(dns::record::deserialize(is));
        }

        if (name == hostname) {
            return records;
        }
    }

    return {};
}
void netkit::dns::standard_cache::store(const std::string& hostname, const std::vector<dns::record>& new_records) {
    std::ifstream is(utility::get_standard_cache_location(), std::ios::binary);
    std::vector<std::pair<std::string, std::vector<netkit::dns::record>>> cache;

    if (is) {
        while (is.peek() != EOF) {
            std::string name = utility::read_string(is);
            uint32_t count = 0;
            utility::read(is, count);

            std::vector<netkit::dns::record> records;
            records.reserve(count);
            for (uint32_t i = 0; i < count; ++i) {
                auto rec = netkit::dns::record::deserialize(is);

                const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()
                ).count();

                if (now_ms < rec.created_at + static_cast<int64_t>(rec.ttl) * 1000) {
                    records.push_back(std::move(rec));
                }
            }

            cache.emplace_back(std::move(name), std::move(records));
        }
    }

    auto it = std::ranges::find_if(cache, [&](const auto& pair) {
        return pair.first == hostname;
    });

    std::vector<netkit::dns::record> merged = new_records;

    if (it != cache.end()) {
        const auto& existing = it->second;

        for (const auto& rec : existing) {
            auto dup = std::ranges::find_if(new_records, [&](const auto& r) {
                return r.name == rec.name &&
                       r.type == rec.type &&
                       r.record_class == rec.record_class &&
                       r.data == rec.data;
            });

            if (dup == new_records.end()) {
                merged.push_back(rec);
            }
        }

        it->second = std::move(merged);
    } else {
        cache.emplace_back(hostname, merged);
    }

    std::ofstream os(utility::get_standard_cache_location(), std::ios::binary | std::ios::trunc);
    if (!os) {
        throw std::runtime_error("failed to open DNS cache file for writing");
    }

    for (const auto& [name, records] : cache) {
        utility::write_string(os, name);
        utility::write<uint32_t>(os, static_cast<uint32_t>(records.size()));
        for (const auto& rec : records) {
            rec.serialize(os);
        }
    }
}