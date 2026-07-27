/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file addr.cpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Implementation of the sock_addr class.
 */
#include <cstring>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <netkit/definitions.hpp>
#include <netkit/dns/nameserver_list.hpp>
#include <netkit/dns/record_type.hpp>
#include <netkit/dns/sync_resolver.hpp>
#include <netkit/except.hpp>
#include <netkit/network/utility.hpp>
#include <netkit/socket/addr.hpp>
#include <utility>

#ifdef NETKIT_DKP
#include <netdb.h>
#include <network.h>
#endif

#if defined(NETKIT_UNIX) && !defined(NETKIT_DKP)
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

#ifdef NETKIT_WINDOWS
#include <ws2tcpip.h>
#endif

/* solely for use internally */
#ifndef NETKIT_DKP
#ifdef NETKIT_ENABLE_SOCK_CUSTOM_RESOLVER
[[nodiscard]] static netkit::network::ip_list get_a_aaaa_from_hostname(const std::string& hostname) {
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
#else
[[nodiscard]] static netkit::network::ip_list get_a_aaaa_from_hostname(const std::string& hostname) {
	if (hostname == "localhost") {
		return {NETKIT_LOCALHOST_IPV4, NETKIT_LOCALHOST_IPV6};
	}

	addrinfo hints{};
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	addrinfo* result = nullptr;

	int res = getaddrinfo(hostname.c_str(), nullptr, &hints, &result);
	if (res != 0) {
#ifdef _WIN32
		throw netkit::dns_error("getaddrinfo failed: " + std::to_string(res));
#else
		throw netkit::dns_error(gai_strerror(res));
#endif
	}

	std::string v4{};
	std::string v6{};

	for (addrinfo* ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
		char buffer[INET6_ADDRSTRLEN] = {0};

		if (ptr->ai_family == AF_INET) {
			auto* ipv4 = reinterpret_cast<sockaddr_in*>(ptr->ai_addr);
			inet_ntop(AF_INET, &ipv4->sin_addr, buffer, sizeof(buffer));
			v4 = buffer;
		}
		else if (ptr->ai_family == AF_INET6) {
			auto* ipv6 = reinterpret_cast<sockaddr_in6*>(ptr->ai_addr);
			inet_ntop(AF_INET6, &ipv6->sin6_addr, buffer, sizeof(buffer));
			v6 = buffer;
		}
	}

	freeaddrinfo(result);

	if (v4.empty() && v6.empty()) {
		throw netkit::dns_error("no valid A or AAAA records found for hostname: " + hostname);
	}

	return {v4, v6};
}
#endif
#endif

netkit::sock::addr::addr(const std::string& hostname, int port, addr_type t) :
    hostname(hostname), port(port), type(t) {

#ifdef NETKIT_DKP
	static std::once_flag flag;
	std::call_once(flag, [] {
		s32 ret;

		char localip[16] = {0};
		char gateway[16] = {0};
		char netmask[16] = {0};

		ret = if_config ( localip, netmask, gateway, true, 20);
		if (ret < 0) {
			throw socket_error("failed to get local network interface address");
		}
	});
#elif NETKIT_WINDOWS
	static std::once_flag wsa_flag;

	std::call_once(wsa_flag, [] {
		WSADATA wsa;
		int res = WSAStartup(MAKEWORD(2, 2), &wsa);
		if (res != 0) {
			throw netkit::socket_error("WSAStartup failed");
		}
	});
#endif

#ifndef NETKIT_DKP
    const auto resolve_host = [](const std::string& h, bool t) -> std::string {
    	auto ip_list = get_a_aaaa_from_hostname(h);
    	auto ip = t ? ip_list.get_ipv6() : ip_list.get_ipv4();

    	return ip;
    };

    if (type == addr_type::hostname) {
    	auto ip6 = resolve_host(hostname, true);
    	auto ip4 = resolve_host(hostname, false);

    	if (!ip6.empty() && netkit::network::usable_ipv6_address_exists()) {
    		ip = ip6;
    		type = addr_type::ipv6;
    	} else if (!ip4.empty()) {
    		ip = ip4;
    		type = addr_type::ipv4;
    	} else {
    		throw ip_error("sock_addr(): could not resolve hostname");
    	}
    } else if (type == addr_type::hostname_ipv4) {
        ip = resolve_host(hostname, false);
        type = netkit::sock::addr_type::ipv4;
#else
	if (type == addr_type::hostname || type == addr_type::hostname_ipv4) {
		netkit::network::ip_list result;
		hostent* host = gethostbyname(hostname.c_str());

		if (!host) {
			throw netkit::dns_error("failed to resolve hostname");
		}

		if (host->h_addrtype != AF_INET) {
			throw netkit::dns_error("not an IPv4 result");
		}

		for (int i = 0; host->h_addr_list[i] != nullptr; i++) {
			in_addr addr{};
			memcpy(&addr, host->h_addr_list[i], sizeof(addr));

			const char* _ip = inet_ntoa(addr);
			if (_ip)
				result.set_ipv4(_ip);

			break;
		}

		ip = result.get_ipv4();
		type = netkit::sock::addr_type::ipv4;
#endif
#ifndef NETKIT_DKP
    } else if (type == addr_type::hostname_ipv6) {
        ip = resolve_host(hostname, true);
        type = netkit::sock::addr_type::ipv6;
#endif
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

#ifndef NETKIT_DKP
netkit::sock::addr::addr(std::filesystem::path path) : path(std::move(path)), type(addr_type::filename) {}
#endif

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