/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file nameserver_list.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides a class representing a list of DNS nameservers, and a function to retrieve system nameservers (when available).
 */
#pragma once

#include <vector>
#include <string>

#include <netkit/except.hpp>

namespace netkit::dns {
	struct nameserver {
		std::string ip;
		std::string sni; /* only applicable for dns over tls */
	};

    class nameserver_list {
        std::vector<nameserver> ipv4{};
        std::vector<nameserver> ipv6{};

        friend nameserver_list get_nameservers();
    public:
        nameserver_list() = default;
        nameserver_list(std::vector<nameserver> ipv4, std::vector<nameserver> ipv6)
: ipv4(std::move(ipv4)), ipv6(std::move(ipv6)) {
            if (this->ipv4.empty() && this->ipv6.empty()) {
                throw parsing_error("dns_nameserver(): at least one IP address must be provided");
            }

        	if (this->ipv4.front().ip.empty() && this->ipv6.front().ip.empty()) {
        		throw parsing_error("dns_nameserver(): at least one IP address must be provided");
        	}
        }
    	nameserver_list(const std::vector<std::string>& ipv4, const std::vector<std::string>& ipv6) {
        	for (auto& it : ipv4) {
        		nameserver srv;
        		srv.ip = it;
        		this->ipv4.push_back(std::move(srv));
        	}
        	for (auto& it : ipv6) {
        		nameserver srv;
        		srv.ip = it;
        		this->ipv6.push_back(std::move(srv));
        	}

        	if (this->ipv4.empty() && this->ipv6.empty()) {
        		throw parsing_error("dns_nameserver(): at least one IP address must be provided");
        	}

        	if (this->ipv4.front().ip.empty() && this->ipv6.front().ip.empty()) {
        		throw parsing_error("dns_nameserver(): at least one IP address must be provided");
        	}
        }
        [[nodiscard]] std::vector<nameserver> get_ipv4() const {
            if (!contains_ipv4()) {
                throw parsing_error("dns_nameserver(): no IPv4 addresses available");
            }
            return ipv4;
        }
        [[nodiscard]] std::vector<nameserver> get_ipv6() const {
            if (!contains_ipv6()) {
                throw parsing_error("dns_nameserver(): no IPv6 addresses available");
            }
            return ipv6;
        }
        [[nodiscard]] bool contains_ipv4() const noexcept {
            return !ipv4.empty();
        }
        [[nodiscard]] bool contains_ipv6() const noexcept {
            return !ipv6.empty();
        }
        void push_back_v4(const std::string& ip, const std::string& sni = {}) {
            ipv4.push_back({ip, sni});
        }
        void push_back_v6(const std::string& ip, const std::string& sni = {}) {
            ipv6.push_back({ip, sni});
        }
    };

    nameserver_list get_nameservers();
}