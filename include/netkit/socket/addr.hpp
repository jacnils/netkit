/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file addr.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides a class representing a socket address, which can be an IP address (IPv4 or IPv6), hostname, or file path.
 *  @see netkit::sock::addr
 *  @see netkit::sock::basic_sync_sock
 */
#pragma once

#include <filesystem>
#include <netkit/socket/addr_type.hpp>
#include <string>

#ifdef NETKIT_WINDOWS
#include <ws2tcpip.h>
#elif defined(NETKIT_UNIX) && !defined(NETKIT_DKP)
#include <sys/un.h>
#include <netinet/in.h>
#elif defined(NETKIT_DKP)
#include <network.h>
#endif

namespace netkit::sock {
	using sockaddr_len = int;

    class NETKIT_API addr final {
        std::filesystem::path path{};
        std::string hostname{};
        std::string ip{};
        int port{};
        addr_type type{addr_type::hostname};

    	sockaddr_storage sa_storage_{};

        addr() = default;

    	void prep_sa();
    public:
        /**
         * @brief Constructs a sock_addr object.
         * @param hostname The hostname or IP address to resolve.
         * @param port The port to use.
         * @param t The address type (ipv4, ipv6, hostname_ipv4, hostname_ipv6).
         */
        addr(const std::string& hostname, int port, addr_type t);
#ifndef NETKIT_DKP
        /**
         * @brief Constructs a sock_addr object for a file path.
         * @param path The file path to use.
         * @throws parsing_error if the path does not exist.
         */
        explicit addr(std::filesystem::path path);
#endif
    	addr(const sockaddr* sa, sockaddr_len len);
        /**
         * @brief Check whether the address is IPv4 or IPv6.
         * @return True if the address is IPv4, false if it is IPv6 or invalid.
         */
        [[nodiscard]] bool is_ipv4() const noexcept;
        /**
         * @brief Check whether the address is IPv6.
         * @return True if the address is IPv6, false if it is IPv4 or invalid.
         */
        [[nodiscard]] bool is_ipv6() const noexcept;
        /**
         * @brief Check whether the address is a file path.
         * @return True if the address is a file path, false if it is an IP address, hostname or invalid.
         */
        [[nodiscard]] bool is_file_path() const noexcept;
        /**
         * @brief Get the stored IP address.
         * @return The stored IP address.
         */
        [[nodiscard]] std::string get_ip() const;
        /**
         * @brief Get the stored file path.
         * @return The stored file path.
         */
        [[nodiscard]] std::filesystem::path get_path() const;
        /**
         * @brief Get the stored hostname.
         * @return The stored hostname.
         */
        [[nodiscard]] std::string get_hostname() const;
        /**
         * @brief Get the stored port.
         * @return The stored port.
         */
        [[nodiscard]] int get_port() const;
		/**
		 * @brief Get the stored type.
		 * @return The stored type.
		 */
		[[nodiscard]] addr_type get_type() const;
        ~addr() = default;

    	[[nodiscard]] const sockaddr* get_sa() const noexcept;
    	[[nodiscard]] sockaddr_len get_sa_len() const noexcept;
    };
}
