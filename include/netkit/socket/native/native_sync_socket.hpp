/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file native_sync_sock.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides a synchronous socket class implementing the basic_native_sync_sock interface.
 */
#pragma once

#include <memory>

#include <netkit/definitions.hpp>

#ifdef NETKIT_WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#elif NETKIT_UNIX
#include <sys/socket.h>
#endif

#include <netkit/socket/native/basic_native_sync_socket.hpp>
#include <netkit/socket/addr.hpp>
#include <netkit/socket/addr_type.hpp>

namespace netkit::socket::native {
    class NETKIT_API native_sync_socket : public basic_native_sync_socket {
        addr addr_;
        type type_{};
        fd_t sockfd{};

        bool bound{false};
        mutable std::string old_bytes;
    public:
        /**
         * @brief Constructs a sync_sock object.
         * @param addr The socket address to bind to.
         * @param t The socket type (tcp, udp, unix).
         * @param opts The socket options (reuse_addr, no_reuse_addr).
         */
        native_sync_socket(const socket::addr& addr, socket::type t, opt opts = opt::reuse_addr|opt::no_delay|opt::blocking);
        /**
         * @brief Constructs a sync_sock object from an existing file descriptor.
         * @param existing_fd The existing file descriptor.
         * @param peer The peer address of the socket.
         * @param t The socket type (tcp, udp, unix).
         * @param opts The socket options (reuse_addr, no_reuse_addr).
         */
        native_sync_socket(fd_t existing_fd, const socket::addr& peer, socket::type t, opt opts = opt::reuse_addr|opt::no_delay|opt::blocking);
        ~native_sync_socket() override;
        socket::addr& get_addr() override;
        [[nodiscard]] const socket::addr& get_addr() const override;
    	void connect() override;
        /**
         * @brief Send data to the server.
         * @param buf The data to send.
         * @param len The length of the data.
         * @return The number of bytes sent.
         */
        std::size_t send(const void* buf, std::size_t len) override;
        [[nodiscard]] std::size_t recv(void* buf, std::size_t len) override;
    	std::pair<std::size_t, addr> recvfrom(void* buf, size_t size) override;
    	std::size_t sendto(const void* buf, std::size_t len, const addr& dest) override;

    	void bind() override;
    	void bind(const addr& addr) override;
    	void unbind() noexcept override;

        /**
         * @brief Close the socket.
         */
        void close() noexcept override;
    	[[nodiscard]] fd_t native_handle() const override;
    	void set_sock_opts(opt opts) override;
    	[[nodiscard]] addr get_peer() const override;
    };
}