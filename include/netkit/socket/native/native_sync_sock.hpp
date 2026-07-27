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

#include <netkit/socket/native/basic_native_sync_sock.hpp>
#include <netkit/socket/addr.hpp>
#include <netkit/socket/addr_type.hpp>

namespace netkit::sock::native {
    class NETKIT_API native_sync_sock : public basic_native_sync_sock {
        addr addr_;
        type type_{};
#ifdef NETKIT_WINDOWS
        fd_t sockfd{INVALID_SOCKET};
#else
        fd_t sockfd{-1};
#endif
        sockaddr_storage sa_storage{};
        bool bound{false};
        mutable std::string old_bytes;

        [[nodiscard]] const sockaddr* get_sa() const;
        [[nodiscard]] socklen_t get_sa_len() const;
    	void prep_sa();
#ifdef NETKIT_DKP
    	sockaddr_storage peer_addr{};
    	bool has_peer{false};
#endif
    public:
        /**
         * @brief Constructs a sync_sock object.
         * @param addr The socket address to bind to.
         * @param t The socket type (tcp, udp, unix).
         * @param opts The socket options (reuse_addr, no_reuse_addr).
         */
        native_sync_sock(const sock::addr& addr, sock::type t, opt opts = opt::reuse_addr|opt::no_delay|opt::blocking);
        /**
         * @brief Constructs a sync_sock object from an existing file descriptor.
         * @param existing_fd The existing file descriptor.
         * @param peer The peer address of the socket.
         * @param t The socket type (tcp, udp, unix).
         * @param opts The socket options (reuse_addr, no_reuse_addr).
         */
        native_sync_sock(fd_t existing_fd, const sock::addr& peer, sock::type t, opt opts = opt::reuse_addr|opt::no_delay|opt::blocking);
        ~native_sync_sock() override;
        sock::addr& get_addr() override;
        [[nodiscard]] const sock::addr& get_addr() const override;
    	void connect() override;
        /**
         * @brief Send data to the server.
         * @param buf The data to send.
         * @param len The length of the data.
         * @return The number of bytes sent.
         */
        std::size_t send(const void* buf, std::size_t len) override;
        [[nodiscard]] std::size_t recv(void* buf, std::size_t len) override;
        /**
         * @brief Close the socket.
         */
        void close() noexcept override;
    	[[nodiscard]] fd_t native_handle() const override;
    	void set_sock_opts(opt opts) override;
    	[[nodiscard]] addr get_peer() const override;
    };
}