/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file nameserver_list.cpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Implementation of functions to retrieve system DNS nameservers.
 */
#include <filesystem>
#include <fstream>

#include <netkit/dns/nameserver_list.hpp>
#include <netkit/definitions.hpp>
#include <netkit/network/utility.hpp>

#ifdef NETKIT_WINDOWS
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include <unordered_set>

#pragma comment(lib, "IPHLPAPI.lib")
#endif
#ifdef NETKIT_MACOS
#include <SystemConfiguration/SystemConfiguration.h>
#endif

namespace netkit::dns {
#if defined(NETKIT_UNIX) && !defined(NETKIT_MACOS)
    nameserver_list get_nameservers() {
        if (!std::filesystem::exists("/etc/resolv.conf")) {
            throw parsing_error("nameserver(): /etc/resolv.conf does not exist");
        }
        std::ifstream file("/etc/resolv.conf");
        if (!file.is_open()) {
            throw parsing_error("failed to open /etc/resolv.conf");
        }

        std::vector<std::string> ipv4_addrs;
        std::vector<std::string> ipv6_addrs;

        std::string line;
        while (std::getline(file, line)) {
            size_t start = line.find_first_not_of(" \t");
            if (start == std::string::npos) continue;
            if (line.compare(start, 10, "nameserver") != 0) continue;

            std::istringstream iss(line.substr(start));
            std::string keyword, ip;
            iss >> keyword >> ip;

            if (!ip.empty() && ip.front() == '[' && ip.back() == ']') {
                ip = ip.substr(1, ip.size() - 2);
            }

            if (ip.empty()) continue;
            if (netkit::network::is_ipv4(ip)) {
                ipv4_addrs.push_back(ip);
            } else if (netkit::network::is_ipv6(ip)) {
                ipv6_addrs.push_back(ip);
            } else {
                continue;
            }
        }

        return {std::move(ipv4_addrs), std::move(ipv6_addrs)};
    }
#endif
#ifdef NETKIT_MACOS
    nameserver_list get_nameservers() {
    	return nameserver_list{
    		{
    			NETKIT_FALLBACK_IPV4_DNS_1, NETKIT_FALLBACK_IPV4_DNS_2,
    			"8.8.8.8", "8.8.4.4"
    			},
    		{NETKIT_FALLBACK_IPV6_DNS_1, NETKIT_FALLBACK_IPV6_DNS_2,
    			"2001:4860:4860::8888", "2001:4860:4860::8844"
				},
    	};
    }
#endif
#ifdef NETKIT_WINDOWS
	nameserver_list get_nameservers() {
		std::vector<std::string> ipv4_addrs;
		std::vector<std::string> ipv6_addrs;

		ULONG bufsiz = 0;
		DWORD result = GetAdaptersAddresses(
			AF_UNSPEC,
	        GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_SKIP_ANYCAST |
		    GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_FRIENDLY_NAME,
			nullptr,
			nullptr,
			&bufsiz
		);

		if (result != ERROR_BUFFER_OVERFLOW) {
			throw parsing_error("failed to get adapter buffer size");
		}

		std::vector<char> buffer(bufsiz);
		auto* adapters =
			reinterpret_cast<IP_ADAPTER_ADDRESSES*>(buffer.data());

		result = GetAdaptersAddresses(
			AF_UNSPEC,
			GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_SKIP_ANYCAST |
			GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_FRIENDLY_NAME,
			nullptr,
			adapters,
			&bufsiz
		);

		if (result != NO_ERROR) {
			throw parsing_error("GetAdaptersAddresses failed");
		}

		std::unordered_set<std::string> seen;

		for (auto* adapter = adapters; adapter != nullptr; adapter = adapter->Next) {
			if (adapter->OperStatus != IfOperStatusUp)
				continue;

			if (adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK)
				continue;

			switch (adapter->IfType) {
				case IF_TYPE_TUNNEL:
				case IF_TYPE_PROP_VIRTUAL:
					continue;
				default:;
				}

			for (auto* dns = adapter->FirstDnsServerAddress; dns; dns = dns->Next) {
				sockaddr* sa = dns->Address.lpSockaddr;

				char ipstr[INET6_ADDRSTRLEN] = {};

				if (sa->sa_family == AF_INET) {
					auto* sin = reinterpret_cast<sockaddr_in*>(sa);
					inet_ntop(AF_INET, &sin->sin_addr, ipstr, sizeof(ipstr));

					if (!network::is_ipv4(ipstr))
						continue;

					std::string ip(ipstr);
					if (!seen.insert(ip).second) continue;
					ipv4_addrs.emplace_back(std::move(ip));
				} else if (sa->sa_family == AF_INET6) {
					auto* sin6 = reinterpret_cast<sockaddr_in6*>(sa);
					inet_ntop(AF_INET6, &sin6->sin6_addr, ipstr, sizeof(ipstr));

					if (!network::is_ipv6(ipstr))
						continue;

					std::string ip(ipstr);

					if (sin6->sin6_scope_id != 0) {
						ip += "%" + std::to_string(sin6->sin6_scope_id);
					}

					if (!seen.insert(ip).second) continue;
					ipv6_addrs.emplace_back(std::move(ip));
				}
			}
		}

		nameserver_list list;
		list.ipv4 = std::move(ipv4_addrs);
		list.ipv6 = std::move(ipv6_addrs);
		return list;
	}
#endif
}