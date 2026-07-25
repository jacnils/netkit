/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file async_sock.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides an asynchronous socket class implementing the basic_async_sock interface.
 */
#pragma once

#include <netkit/definitions.hpp>

#ifdef NETKIT_LINUX

#include <sys/socket.h>

#include <netkit/sock/addr.hpp>
#include <netkit/sock/addr_type.hpp>
#include <netkit/sock/basic_async_sock.hpp>

#include <memory>

namespace netkit::sock {
	constexpr fd_t INVALID_SOCKET = -1;

    class NETKIT_API async_sock : public basic_async_sock {
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
        async_sock(io::io_context& ctx, const sock::addr& addr, sock::type t, opt opts = opt::no_reuse_addr|opt::no_delay);
        async_sock(io::io_context& ctx, fd_t existing_fd, const sock::addr& peer, sock::type t, opt opts = opt::no_reuse_addr|opt::no_delay);
    	~async_sock() override;
        sock::addr& get_addr() override;
        [[nodiscard]] const sock::addr& get_addr() const override;
        netkit::io::task<void> connect() override;

        void bind() override;
        void unbind() override;
        void listen(int backlog) override;
        void listen() override;
        [[nodiscard]] netkit::io::task<std::unique_ptr<basic_async_sock>> accept() override;
        netkit::io::task<std::size_t> send(const void* buf, size_t len) override;
    	[[nodiscard]] netkit::io::task<std::size_t> recv(void* buf, std::size_t size) override;
        void close() override;
        [[nodiscard]] sock::addr get_peer() const override;
    	[[nodiscard]] fd_t native_handle() const override;
    	void set_sock_opts(opt opts) override;
    };
}

#endif