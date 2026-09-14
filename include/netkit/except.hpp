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

#include <exception>
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

    /**
     * @brief Thrown for TLS/SSL failures (handshake, context/session setup, certificate export, etc.).
     * @note Not guarded behind a TLS backend macro: the class itself has no backend dependency,
     *       only the code paths that throw it (e.g. src/stream/wolfssl/*, src/stream/openssl/*) do.
     */
    class NETKIT_API ssl_error : public generic_error {
    public:
        ssl_error() : generic_error("SSL error") {}
        explicit ssl_error(std::string msg) : generic_error(std::move(msg)) {}
    };

    /**
     * @brief Thrown when netkit is used incorrectly: an object is used in an invalid state,
     *        a precondition/argument is violated, or an operation is not supported.
     * @note Mirrors the role std::logic_error plays in the standard library, but lives in
     *       netkit's own hierarchy so callers only ever need to catch netkit::generic_error.
     */
    class NETKIT_API logic_error : public generic_error {
    public:
        logic_error() : generic_error("Logic error") {}
        explicit logic_error(std::string msg) : generic_error(std::move(msg)) {}
    };

    /**
     * @brief Thrown when an operation would exceed (or exceeded) an explicit size/length bound.
     * @note Mirrors std::length_error, and like it, derives from the "logic error" branch of
     *       the hierarchy since exceeding a caller-specified bound is a precondition violation.
     */
    class NETKIT_API length_error : public logic_error {
    public:
        length_error() : logic_error("Length error") {}
        explicit length_error(std::string msg) : logic_error(std::move(msg)) {}
    };

    /**
     * @brief Thrown for low-level I/O failures that aren't specific to a socket, IP, or DNS
     *        operation: file access, and OS event-loop backends (epoll/kqueue/IOCP/etc.).
     */
    class NETKIT_API io_error : public generic_error {
    public:
        io_error() : generic_error("I/O error") {}
        explicit io_error(std::string msg) : generic_error(std::move(msg)) {}
    };
}