/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file sock_addr.cpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Implementation of the sock_addr class.
 */
#include <filesystem>
#include <netkit/definitions.hpp>
#include <netkit/dns/nameserver_list.hpp>
#include <netkit/dns/record_type.hpp>
#include <netkit/dns/sync_resolver.hpp>
#include <netkit/except.hpp>
#include <netkit/network/utility.hpp>
#include <netkit/sock/addr.hpp>

/* solely for use internally */
[[nodiscard]] static netkit::sock::ip_list get_a_aaaa_from_hostname(const std::string& hostname) {
    if (hostname == "localhost") {
        return {NETKIT_LOCALHOST_IPV4, NETKIT_LOCALHOST_IPV6};
    }
    auto nameservers = netkit::dns::get_nameservers();
    if (nameservers.contains_ipv4() == false && nameservers.contains_ipv6() == false) {
        nameservers = {
            {NETKIT_FALLBACK_IPV4_DNS_1, NETKIT_FALLBACK_IPV4_DNS_2},
            {NETKIT_FALLBACK_IPV6_DNS_1, NETKIT_FALLBACK_IPV6_DNS_2},
        };
    }

    netkit::dns::sync_resolver resolver(nameservers);

    auto records = resolver.query_records(hostname, netkit::dns::record_type::A);
    auto records_v6 = resolver.query_records(hostname, netkit::dns::record_type::AAAA);

    std::string v4{};
    std::string v6{};

    records.insert(records.end(), records_v6.begin(), records_v6.end());

    for (const auto& rec : records) {
        std::visit([&v4, &v6]<typename T0>(T0&& data) {
            using T = std::decay_t<T0>;
            if constexpr (std::is_same_v<T, netkit::dns::a_record_data>) {
                v4 = data.ip.get_ipv4();
            } else if constexpr (std::is_same_v<T, netkit::dns::aaaa_record_data>) {
                v6 = data.ip.get_ipv6();
            }
        }, rec.data);
    }

    if (v4.empty() && v6.empty()) {
        throw netkit::dns_error("no valid A or AAAA records found for hostname: " + hostname);
    }

    return {v4, v6};
}

netkit::sock::addr::addr(const std::string& hostname, int port, addr_type t) :
    hostname(hostname), port(port), type(t) {

    const auto resolve_host = [](const std::string& h, bool t) -> std::string {
        try {
            auto ip_list = get_a_aaaa_from_hostname(h);
            auto ip = t ? ip_list.get_ipv6() : ip_list.get_ipv4();
            return ip;
        } catch (const std::exception&) {
            return {};
        }
    };

    if (type == addr_type::hostname) {
        ip = resolve_host(hostname, true);
        type = netkit::sock::addr_type::ipv6;

        if (!netkit::network::usable_ipv6_address_exists()) {
            ip.clear();
        }

        if (ip.empty()) {
            ip = resolve_host(hostname, false);
            type = netkit::sock::addr_type::ipv4;
        }
    } else if (type == addr_type::hostname_ipv4) {
        ip = resolve_host(hostname, false);
        type = netkit::sock::addr_type::ipv4;
    } else if (type == addr_type::hostname_ipv6) {
        ip = resolve_host(hostname, true);
        type = netkit::sock::addr_type::ipv6;
    } else if (type == addr_type::ipv4 || type == addr_type::ipv6) {
        ip = hostname;
    } else {
        throw ip_error("sock_addr(): invalid address type");
    }

    if (ip.empty()) {
        throw ip_error("sock_addr(): could not resolve hostname or invalid IP address");
    }

    if (!network::is_ipv4(ip) && !network::is_ipv6(ip)) {
        throw parsing_error("sock_addr(): invalid address type (constructor)");
    }

    if (this->hostname == ip) {
        this->hostname.clear();
    }
}

netkit::sock::addr::addr(const std::filesystem::path& path) : path(path), type(addr_type::filename) {
    if (!std::filesystem::exists(path)) {
        throw parsing_error("sock_addr(): path does not exist");
    }
}

bool netkit::sock::addr::is_ipv4() const noexcept {
    return type == addr_type::ipv4;
}

bool netkit::sock::addr::is_ipv6() const noexcept {
    return type == addr_type::ipv6;
}

bool netkit::sock::addr::is_file_path() const noexcept {
    return type == addr_type::filename;
}

std::string netkit::sock::addr::get_ip() const {
    if (type == addr_type::filename) {
        throw parsing_error("sock_addr(): cannot get IP from a file path");
    }

    return this->ip;
}

[[nodiscard]] std::filesystem::path netkit::sock::addr::get_path() const {
    if (type != addr_type::filename) {
        throw parsing_error("sock_addr(): cannot get path from an IP address or hostname");
    }
    return this->path;
}

std::string netkit::sock::addr::get_hostname() const {
    if (hostname.empty()) {
        throw parsing_error("hostname is empty, use get_ip() instead");
    }
    if (type == addr_type::filename) {
        throw parsing_error("sock_addr(): cannot get hostname from a file path");
    }
    return hostname;
}

int netkit::sock::addr::get_port() const {
    if (type == addr_type::filename) {
        throw parsing_error("sock_addr(): cannot get port from a file path");
    }

    return port;
}

netkit::sock::addr_type netkit::sock::addr::get_type() const {
	return type;
}