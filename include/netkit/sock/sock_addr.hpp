/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file sock_addr.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides a class representing a socket address, which can be an IP address (IPv4 or IPv6), hostname, or file path.
 *  @see netkit::sock::sock_addr
 *  @see netkit::sock::basic_sync_sock
 */
#pragma once

#include <filesystem>
#include <string>
#include <netkit/sock/sock_addr_type.hpp>

namespace netkit::sock {
    class NETKIT_API sock_addr final {
        std::filesystem::path path{};
        std::string hostname{};
        std::string ip{};
        int port{};
        sock_addr_type type{sock_addr_type::hostname};
        friend sock_addr get_peer(sock_fd_t);

        sock_addr() = default;
    public:
        /**
         * @brief Constructs a sock_addr object.
         * @param hostname The hostname or IP address to resolve.
         * @param port The port to use.
         * @param t The address type (ipv4, ipv6, hostname_ipv4, hostname_ipv6).
         */
        sock_addr(const std::string& hostname, int port, sock_addr_type t);
        /**
         * @brief Constructs a sock_addr object for a file path.
         * @param path The file path to use.
         * @throws parsing_error if the path does not exist.
         */
        explicit sock_addr(const std::filesystem::path& path);
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
        ~sock_addr() = default;
    };
}
