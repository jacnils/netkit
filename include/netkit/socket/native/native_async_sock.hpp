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
#include <netkit/socket/native/basic_native_async_sock.hpp>
#include <netkit/socket/addr.hpp>
#include <netkit/socket/addr_type.hpp>

#ifdef NETKIT_UNIX
#include <sys/socket.h>
#elifdef NETKIT_WINDOWS
#include <ws2tcpip.h>
#endif

namespace netkit::sock::native {
#ifndef NETKIT_WINDOWS
constexpr fd_t INVALID_SOCKET = -1;
#endif
    class NETKIT_API native_async_sock : public basic_native_async_sock {
        addr addr_;
        type type_{};
        fd_t sockfd{INVALID_SOCKET};

        sockaddr_storage sa_storage{};
        bool bound{false};

        [[nodiscard]] const sockaddr* get_sa() const;
        [[nodiscard]] socklen_t get_sa_len() const;
    	void prep_sa();

		io::io_context& context_;
    public:
        native_async_sock(io::io_context& ctx, const sock::addr& addr, sock::type t, opt opts = opt::reuse_addr|opt::no_delay|opt::no_blocking);
        native_async_sock(io::io_context& ctx, fd_t existing_fd, const sock::addr& peer, sock::type t, opt opts = opt::reuse_addr|opt::no_delay|opt::no_blocking);
    	~native_async_sock() override;
        sock::addr& get_addr() override;
        [[nodiscard]] const sock::addr& get_addr() const override;
        netkit::io::task<void> connect() override;

        netkit::io::task<std::size_t> send(const void* buf, size_t len) override;
    	[[nodiscard]] netkit::io::task<std::size_t> recv(void* buf, std::size_t size) override;
        void close() noexcept override;
        [[nodiscard]] sock::addr get_peer() const override;
    	[[nodiscard]] fd_t native_handle() const override;
    	void set_sock_opts(opt opts) override;
    };
}