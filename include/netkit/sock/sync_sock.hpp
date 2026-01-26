/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file sync_sock.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides a synchronous socket class implementing the basic_sync_sock interface.
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

#include <netkit/sock/sock_addr_type.hpp>
#include <netkit/sock/sock_addr.hpp>
#include <netkit/sock/basic_sync_sock.hpp>

namespace netkit::sock {
    class NETKIT_API sync_sock : public basic_sync_sock {
        sock_addr addr;
        sock_type type{};
#ifdef NETKIT_WINDOWS
        sock_fd_t sockfd{INVALID_SOCKET};
#else
        sock_fd_t sockfd{-1};
#endif
        sockaddr_storage sa_storage{};
        bool bound{false};
        mutable std::string old_bytes;

        [[nodiscard]] const sockaddr* get_sa() const;
        [[nodiscard]] socklen_t get_sa_len() const;
    	void prep_sa();
        void set_sock_opts(sock_opt opts) const;
    public:
        /**
         * @brief Constructs a sync_sock object.
         * @param addr The socket address to bind to.
         * @param t The socket type (tcp, udp, unix).
         * @param opts The socket options (reuse_addr, no_reuse_addr).
         */
#ifdef NETKIT_UNIX
        sync_sock(const sock_addr& addr, sock_type t, sock_opt opts = sock_opt::no_reuse_addr|sock_opt::no_delay|sock_opt::blocking);
        /**
         * @brief Constructs a sync_sock object from an existing file descriptor.
         * @param existing_fd The existing file descriptor.
         * @param peer The peer address of the socket.
         * @param t The socket type (tcp, udp, unix).
         * @param opts The socket options (reuse_addr, no_reuse_addr).
         */
        sync_sock(int existing_fd, const sock_addr& peer, sock_type t, sock_opt opts = sock_opt::no_reuse_addr|sock_opt::no_delay|sock_opt::blocking);
#endif
#ifdef NETKIT_WINDOWS
        sync_sock(const sock_addr& addr, sock_type t, sock_opt opts = sock_opt::no_reuse_addr|sock_opt::no_delay|sock_opt::blocking);
#endif
        ~sync_sock() override;
        sock_addr& get_addr() override;
        [[nodiscard]] const sock_addr& get_addr() const override;
        void connect() override;
        /**
         * @brief Bind the socket to the address.
         */
        void bind() override;
        /**
         * @brief Unbind the socket from the address.
         */
        void unbind() override;
        /**
         * @brief Listen for incoming connections.
         * @param backlog The maximum number of pending connections.
         * @note Very barebones, use with care.
         */
        void listen(int backlog) override;
        /**
         * @brief Listen for incoming connections with default backlog.
         * @note Uses SOMAXCONN as the default backlog value.
         */
        void listen() override;
        /**
         * @brief Accept a connection from a client.
         * @return sock_handle The socket handle for the accepted connection.
         */
        [[nodiscard]] std::unique_ptr<basic_sync_sock> accept() override;
        /**
         * @brief Send data to the server.
         * @param buf The data to send.
         * @param len The length of the data.
         * @return The number of bytes sent.
         */
        int send(const void* buf, size_t len) override;
        /**
         * @brief Send a string to the server.
         * @param buf The string to send.
         */
        void send(const std::string& buf) override;
        /**
         * @brief Returns bytes that were read, further than the requested length (as defined by the eof parameter in recv()).
         * @note This does NOT need to be called if you intend to call recv() again, as recv() prepends these bytes automatically.
         * @note Call clear_overflow_bytes() after calling, if you do not want recv() to use these bytes again.
         * @return std::string of overflow bytes.
         */
        [[nodiscard]] std::string overflow_bytes() const override;
        /**
         * @brief Clear the overflow bytes buffer.
         * @note This does NOT need to be called if you intend to call recv() again, as recv() prepends these bytes automatically.
         */
        void clear_overflow_bytes() const override;
        /**
         * @brief Receive data from the server.
         * @param timeout_seconds The timeout in seconds (-1 means wait indefinitely until match is found)
         * @param match The substring to look for in received data.
         * @param eof The number of bytes to read before considering the match complete.
         * @return The received data as a sock_recv_result object.
         */
        [[nodiscard]] sock_recv_result recv(int timeout_seconds, const std::string& match, size_t eof) override;
        [[nodiscard]] sock_recv_result primitive_recv() override;

        /* @brief Receive data from the server.
         * @param timeout_seconds The timeout in seconds (-1 means wait indefinitely).
         * @return The received data as a sock_recv_result
         */
        [[nodiscard]] sock_recv_result recv(int timeout_seconds) override;
        /**
         * @brief Receive data from the server until a specific match is found.
         * @param timeout_seconds The timeout in seconds (-1 means wait indefinitely).
         * @param match The substring to look for in received data.
         * @return The received data as a sock_recv_result object.
         */
        [[nodiscard]] sock_recv_result recv(int timeout_seconds, const std::string& match) override;
        /**
         * @brief Receive data from the server until a specific match is found or a certain amount of data is received.
         * @param timeout_seconds The timeout in seconds (-1 means wait indefinitely).
         * @param eof The number of bytes to read before considering the match complete.
         * @return The received data as a sock_recv_result object.
         */
        [[nodiscard]] sock_recv_result recv(int timeout_seconds, size_t eof) override;
        /**
         * @brief Close the socket.
         */
        void close() override;
        [[nodiscard]] sock_addr get_peer() const override;
    };
}