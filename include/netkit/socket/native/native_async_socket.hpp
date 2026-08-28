/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file native_async_sock.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides an asynchronous socket class implementing the basic_native_async_sock interface.
 */
#pragma once

#include <netkit/definitions.hpp>

#include <memory>
#include <netkit/socket/native/basic_native_async_socket.hpp>
#include <netkit/socket/addr.hpp>
#include <netkit/socket/addr_type.hpp>

#ifdef NETKIT_UNIX
#include <sys/socket.h>
#elifdef NETKIT_WINDOWS
#include <ws2tcpip.h>
#endif

namespace netkit::socket::native {
    class NETKIT_API native_async_socket : public basic_native_async_socket {
        addr addr_;
        type type_{};
        fd_t sockfd{};

        bool bound{false};

		io::io_context& context_;
    public:
        native_async_socket(io::io_context& ctx, const socket::addr& addr, socket::type t, opt opts = opt::reuse_addr|opt::no_delay|opt::no_blocking);
        native_async_socket(io::io_context& ctx, fd_t existing_fd, const socket::addr& peer, socket::type t, opt opts = opt::reuse_addr|opt::no_delay|opt::no_blocking);
    	~native_async_socket() override;
        socket::addr& get_addr() override;
        [[nodiscard]] const socket::addr& get_addr() const override;
        netkit::io::task<void> connect() override;

    	void bind() override;
    	void bind(const addr& addr) override;
    	void unbind() noexcept override;

        netkit::io::task<std::size_t> send(const void* buf, size_t len) override;
    	[[nodiscard]] netkit::io::task<std::size_t> recv(void* buf, std::size_t size) override;
        void close() noexcept override;
        [[nodiscard]] socket::addr get_peer() const override;
    	[[nodiscard]] fd_t native_handle() const override;
		netkit::io::task<std::pair<std::size_t, netkit::socket::addr>> recvfrom(void*	buf, size_t size) override;
		netkit::io::task<std::size_t> sendto(const void* buf, std::size_t len, const addr& dest) override;

		void set_sock_opts(opt opts) override;
    };
}