/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file query_builder.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides a synchronous DNS resolver class template and interface.
 */
#pragma once

#include <netkit/dns/cache.hpp>
#include <netkit/dns/nameserver_list.hpp>
#include <netkit/dns/query_builder.hpp>
#include <netkit/dns/record_type.hpp>
#include <netkit/dns/response_parser.hpp>
#include <netkit/network/utility.hpp>
#include <netkit/sock/addr.hpp>
#include <netkit/sock/addr_type.hpp>
#include <netkit/sock/sync_sock.hpp>

#ifdef NETKIT_UNIX
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

namespace netkit::dns {
    template <typename T = standard_cache>
    class basic_sync_resolver {
    public:
        [[nodiscard]] virtual std::vector<dns::record> query_records(const std::string& hostname, dns::record_type type) const = 0;
        virtual ~basic_sync_resolver() = default;
    };

	template <typename T = standard_cache>
    class sync_resolver : public basic_sync_resolver<T> {
        nameserver_list list{};

        void throw_if_invalid() const {
            if (list.contains_ipv4() || list.contains_ipv6()) {
                return;
            }
            throw parsing_error("sync_dns_resolver(): at least one IP address must be provided");
        }
    public:
        explicit sync_resolver(nameserver_list list) : list(std::move(list)) {
            throw_if_invalid();
        }
        sync_resolver() : list(get_nameservers()) {
            throw_if_invalid();
        }

        [[nodiscard]] std::vector<dns::record> query_records(const std::string& hostname, record_type type) const override {
		    throw_if_invalid();

			T cache{};
			auto cached_records = cache.lookup(hostname);

			const auto now = std::chrono::system_clock::now();
			std::vector<dns::record> valid_cached;

			for (const auto& r : cached_records) {
				if (r.type != type) continue;

				auto created = std::chrono::system_clock::time_point(
				std::chrono::milliseconds(r.created_at));
				auto expires = created + std::chrono::seconds(r.ttl);

				if (expires > now) valid_cached.push_back(r);
			}

			if (!valid_cached.empty())
				return valid_cached;


			query_builder builder;
			builder.add_question(hostname, type);

			std::vector<uint8_t> query = builder.build();

			std::vector<dns::record> all_records;

			auto send_udp = [&query](const std::string &server,
                                netkit::sock::addr_type family) -> std::optional<std::vector<uint8_t> > {
				netkit::sock::addr addr(server, 53, family);
				netkit::sock::sync_sock sock(
					addr,
					netkit::sock::type::udp,
					netkit::sock::opt::blocking |
					netkit::sock::opt::no_delay
				);

				sock.connect();

		        sock.send(query.data(), query.size());

				auto resp = sock.recv(2, 4096).data;

				if (resp.size() < 12)
					return std::nullopt;

				if ((resp[2] & 0x02) != 0)
					return std::nullopt;

				return std::vector<uint8_t>(resp.begin(), resp.end());
			};


			auto send_tcp = [&](const std::string& server, netkit::sock::addr_type family) -> std::optional<std::vector<uint8_t>> {
				netkit::sock::addr addr(server, 53, family);
				netkit::sock::sync_sock sock(
					addr,
					netkit::sock::type::tcp,
					netkit::sock::opt::blocking |
					netkit::sock::opt::no_delay
				);
				sock.connect();

				uint16_t len = htons(static_cast<uint16_t>(query.size()));
				sock.send(reinterpret_cast<char *>(&len), 2);
				sock.send(reinterpret_cast<char *>(query.data()), query.size());

				std::string lenbuf;
				while (lenbuf.size() < 2) {
					auto chunk = sock.recv(2, 2 - lenbuf.size()).data;
					if (chunk.empty())
						return std::nullopt;
					lenbuf += chunk;
				}

				uint16_t resp_len = ntohs(*reinterpret_cast<const uint16_t*>(lenbuf.data()));
				if (resp_len == 0)
					return std::nullopt;

				std::string resp;
				resp.reserve(resp_len);

				while (resp.size() < resp_len) {
					size_t to_read = resp_len - resp.size();
					auto chunk = sock.recv(2, to_read).data;
					if (chunk.empty())
						return std::nullopt;
					resp += chunk;
				}

				return std::vector<uint8_t>(resp.begin(), resp.end());
			};

			auto try_server = [&](const std::string& server, netkit::sock::addr_type family) -> bool {
				auto udp_resp = send_udp(server, family);
				std::optional<std::vector<uint8_t>> final_resp;

				if (udp_resp.has_value()) {
					final_resp = udp_resp;
				} else {
					auto tcp_resp = send_tcp(server, family);
					if (!tcp_resp.has_value())
						return false;

					final_resp = tcp_resp;
				}

				response_parser parser(*final_resp);
				auto recs = parser.parse();

				all_records.insert(all_records.end(), recs.begin(), recs.end());

				return !recs.empty();
			};

			bool success = false;

			if (network::usable_ipv6_address_exists() && list.contains_ipv6()) {
				for (const auto& s : list.get_ipv6()) {
					if (try_server(s, netkit::sock::addr_type::ipv6)) {
						success = true;
						break;
					}
				}
			}

			if (!success && list.contains_ipv4()) {
				for (const auto& s : list.get_ipv4()) {
					if (try_server(s, netkit::sock::addr_type::ipv4)) {
						success = true;
						break;
					}
				}
			}

			if (!success)
				throw dns_error("all DNS queries failed.");

			if (all_records.empty())
				throw dns_error("no DNS records found for: " + hostname);

			cache.store(hostname, all_records);

			return all_records;
		}
    };
}
