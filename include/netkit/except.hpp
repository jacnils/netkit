/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file except.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides exception classes used throughout the Netkit library.
 */
#pragma once

#include <string>
#include <utility>

#include <netkit/export.hpp>

namespace netkit {
    using exception_type = std::exception;

    class NETKIT_API generic_error : public exception_type {
    protected:
        std::string message;
    public:
        explicit generic_error(std::string msg) : message(std::move(msg)) {}
        [[nodiscard]] const char* what() const noexcept override { return message.c_str(); }
    };

    class NETKIT_API socket_error : public generic_error {
    public:
        socket_error() : generic_error("Socket error") {}
        explicit socket_error(std::string msg) : generic_error(std::move(msg)) {}
    };

    class NETKIT_API parsing_error : public generic_error {
    public:
        parsing_error() : generic_error("Parsing error") {}
        explicit parsing_error(std::string msg) : generic_error(std::move(msg)) {}
    };

    class NETKIT_API ip_error : public generic_error {
    public:
        ip_error() : generic_error("IP error") {}
        explicit ip_error(std::string msg) : generic_error(std::move(msg)) {}
    };

    class NETKIT_API dns_error : public generic_error {
    public:
        dns_error() : generic_error("DNS error") {}
        explicit dns_error(std::string msg) : generic_error(std::move(msg)) {}
    };

#ifdef NETKIT_OPENSSL
    class NETKIT_API ssl_error : public generic_error {
    public:
        ssl_error() : generic_error("SSL error") {}
        explicit ssl_error(std::string msg) : generic_error(std::move(msg)) {}
    };
#endif
}