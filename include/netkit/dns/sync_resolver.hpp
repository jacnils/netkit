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

#ifdef NETKIT_DNS

#include <netkit/dns/cache.hpp>
#include <netkit/dns/nameserver_list.hpp>
#include <netkit/dns/query_builder.hpp>
#include <netkit/dns/record_type.hpp>
#include <netkit/dns/response_parser.hpp>
#include <netkit/network/utility.hpp>
#include <netkit/socket/addr.hpp>
#include <netkit/socket/addr_type.hpp>
#include <netkit/udp/udp_datagram.hpp>
#include <netkit/tcp/tcp_stream.hpp>
#include <netkit/stream/tls_stream.hpp>

#ifdef NETKIT_UNIX
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

#ifndef NETKIT_DKP

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
		bool tls{};

        void throw_if_invalid() const {
            if (list.contains_ipv4() || list.contains_ipv6()) {
                return;
            }
            throw parsing_error("sync_dns_resolver(): at least one IP address must be provided");
        }
    public:
        explicit sync_resolver(nameserver_list list, bool use_dot = false) : list(std::move(list)), tls(use_dot) {
            throw_if_invalid();
        }
        sync_resolver(bool use_dot = false) : list(get_nameservers()), tls(use_dot) {
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

        	auto send_udp = [&query](const std::string& server, netkit::socket::addr_type family) -> std::optional<std::vector<uint8_t>> {
        		netkit::socket::addr addr(server, 53, family);

        		netkit::udp::udp_datagram sock(addr);

        		std::array<std::byte, 4096> buffer{};

        		sock.send_to(
					std::as_bytes(std::span(query)),
					addr
				);

        		auto [size, from] = sock.recv_from(buffer);

        		if (size < 12)
        			return std::nullopt;

        		uint16_t flags =
					(static_cast<uint8_t>(buffer[2]) << 8) |
					static_cast<uint8_t>(buffer[3]);

        		if (flags & 0x0200)
        			return std::nullopt;

        		return std::vector<uint8_t>(
					reinterpret_cast<uint8_t*>(buffer.data()),
					reinterpret_cast<uint8_t*>(buffer.data()) + size
				);
        	};

        	auto send_tcp = [&query](const std::string& server, netkit::socket::addr_type family) -> std::optional<std::vector<uint8_t>> {
        		netkit::socket::addr addr(server, 53, family);
        		netkit::tcp::tcp_stream sock(addr);

        		sock.connect();

        		std::array<std::byte, 2> lenbuf{
        			std::byte(query.size() >> 8),
					std::byte(query.size() & 0xFF)
				};

        		if (sock.write(lenbuf).status != stream::stream_status::success)
        			return std::nullopt;

        		if (sock.write(std::as_bytes(std::span(query))).status != stream::stream_status::success)
        			return std::nullopt;

        		std::array<std::byte, 2> resp_len_buf{};
        		std::size_t total = 0;

        		while (total < 2) {
        			auto res = sock.read(
						std::span(resp_len_buf).subspan(total)
					);

        			if (res.status != stream::stream_status::success || res.bytes == 0)
        				return std::nullopt;

        			total += res.bytes;
        		}

        		uint16_t resp_len =
					(static_cast<uint16_t>(std::to_integer<uint8_t>(resp_len_buf[0])) << 8) |
					std::to_integer<uint8_t>(resp_len_buf[1]);

        		if (resp_len == 0)
        			return std::nullopt;

        		std::vector<std::byte> resp(resp_len);

        		total = 0;

        		while (total < resp_len) {
        			auto res = sock.read(
						std::span(resp).subspan(total)
					);

        			if (res.status != stream::stream_status::success || res.bytes == 0)
        				return std::nullopt;

        			total += res.bytes;
        		}

        		return std::vector<uint8_t>(
					reinterpret_cast<uint8_t*>(resp.data()),
					reinterpret_cast<uint8_t*>(resp.data()) + resp.size()
				);
        	};

#ifdef NETKIT_TLS_STREAM
        	auto send_tls = [&query](const std::string& server, const std::string& sni, netkit::socket::addr_type family) -> std::optional<std::vector<uint8_t>> {
        		netkit::socket::addr addr(server, 853, family);
        		std::unique_ptr<netkit::tcp::tcp_stream> _sock = std::make_unique<tcp::tcp_stream>(addr);

        		_sock->connect();

        		netkit::stream::tls_stream sock(std::move(_sock),
					netkit::stream::version::TLS_1_2,
					netkit::stream::verification::none,
					{},
					sni
				);

        		sock.perform_handshake();

        		std::array<std::byte, 2> lenbuf{
        			std::byte(query.size() >> 8),
					std::byte(query.size() & 0xFF)
				};

        		if (sock.write(lenbuf).status != stream::stream_status::success)
        			return std::nullopt;

        		if (sock.write(std::as_bytes(std::span(query))).status != stream::stream_status::success)
        			return std::nullopt;

        		std::array<std::byte, 2> resp_len_buf{};
        		std::size_t total = 0;

        		while (total < 2) {
        			auto res = sock.read(
						std::span(resp_len_buf).subspan(total)
					);

        			if (res.status != stream::stream_status::success || res.bytes == 0)
        				return std::nullopt;

        			total += res.bytes;
        		}

        		uint16_t resp_len =
					(static_cast<uint16_t>(std::to_integer<uint8_t>(resp_len_buf[0])) << 8) |
					std::to_integer<uint8_t>(resp_len_buf[1]);

        		if (resp_len == 0)
        			return std::nullopt;

        		std::vector<std::byte> resp(resp_len);

        		total = 0;

        		while (total < resp_len) {
        			auto res = sock.read(
						std::span(resp).subspan(total)
					);

        			if (res.status != stream::stream_status::success || res.bytes == 0)
        				return std::nullopt;

        			total += res.bytes;
        		}

        		return std::vector<uint8_t>(
					reinterpret_cast<uint8_t*>(resp.data()),
					reinterpret_cast<uint8_t*>(resp.data()) + resp.size()
				);
        	};
#else
        	if (tls) {
        		throw netkit::dns_error{"TLS not enabled in netkit"};
        	}
#endif

			auto try_server = [&](const nameserver& server, netkit::socket::addr_type family) -> bool {
				std::optional<std::vector<uint8_t>> final_resp;

#ifdef NETKIT_TLS_STREAM
				if (tls) {
					final_resp = send_tls(server.ip, server.sni, family);
				}
#endif

				if (!final_resp.has_value()) {
					final_resp = send_udp(server.ip, family);
				}

				if (!final_resp.has_value()) {
					final_resp = send_tcp(server.ip, family);

					if (!final_resp.has_value())
						return false;
				}

				response_parser parser(*final_resp);
				auto recs = parser.parse();

				all_records.insert(all_records.end(), recs.begin(), recs.end());

				return !recs.empty();
			};

			bool success = false;

			if (network::usable_ipv6_address_exists() && list.contains_ipv6()) {
				for (const auto& s : list.get_ipv6()) {
					if (s.sni.empty() && tls) {
						continue;
					}

					if (try_server(s, netkit::socket::addr_type::ipv6)) {
						success = true;
						break;
					}
				}
			}

			if (!success && list.contains_ipv4()) {
				for (const auto& s : list.get_ipv4()) {
					if (s.sni.empty() && tls) {
						continue;
					}

					if (try_server(s, netkit::socket::addr_type::ipv4)) {
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

#endif

#endif