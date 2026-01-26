/** netkit
*  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file cache.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides a basic DNS cache interface and a standard implementation.
 */
#pragma once

#include <vector>
#include <string>

#include <netkit/dns/record_type.hpp>

namespace netkit::dns {
    class NETKIT_API basic_cache {
    public:
        virtual ~basic_cache() = default;
        [[nodiscard]] virtual std::vector<dns::record> lookup(const std::string& hostname) const = 0;
        virtual void store(const std::string& hostname, const std::vector<dns::record>& records) = 0;
    };

    class NETKIT_API standard_cache : public basic_cache {
    public:
        [[nodiscard]] std::vector<dns::record> lookup(const std::string& hostname) const override;
        void store(const std::string& hostname, const std::vector<dns::record>& new_records) override;
    };
}