/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file ssl_sync_sock.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides a synchronous SSL/TLS socket class wrapping a basic synchronous socket.
 *  @see netkit::sock::basic_sync_sock
 *  @see netkit::sock::sync_sock
 */
#pragma once

#ifdef NETKIT_WOLFSSL

#include <wolfssl/options.h>
#include <wolfssl/ssl.h>

#include <../native/basic_native_sync_sock.hpp>
#include <memory>
#include <mutex>
#include <netkit/export.hpp>
#include <netkit/socket/addr.hpp>
#include <netkit/socket/ssl_sync_sock_enum.hpp>

namespace netkit::sock {
    class NETKIT_API ssl_sync_sock : public basic_sync_sock {
    public:
        explicit ssl_sync_sock(std::unique_ptr<sock::basic_sync_sock> underlying,
                               mode ssl_mode,
                               version ssl_version = version::TLS_1_2,
                               verification ssl_verification = verification::peer,
                               std::string cert_path = "",
                               std::string key_path = "");

        ~ssl_sync_sock() override;

        void connect() override;
        void bind() override;
        void unbind() override;
        void listen(int backlog) override;
        void listen() override;

        bool is_secure() const;

        [[nodiscard]] std::unique_ptr<basic_sync_sock> accept() override;
    	[[nodiscard]] std::unique_ptr<ssl_sync_sock> accept_explicit_ssl();

        int send(const void* buf, size_t len) override;
        void send(const std::string& buf) override;

        [[nodiscard]] recv_result recv(int timeout_seconds) override;
        [[nodiscard]] recv_result recv(int timeout_seconds, const std::string& match) override;
        [[nodiscard]] recv_result recv(int timeout_seconds, const std::string& match, size_t eof) override;
        [[nodiscard]] recv_result recv(int timeout_seconds, size_t eof) override;
    	[[nodiscard]] recv_result recv() override;

        std::string overflow_bytes() const override;
        void clear_overflow_bytes() const override;

        void close() override;
        void perform_handshake();

        [[nodiscard]] netkit::sock::addr get_peer() const override;
    	addr& get_addr() override;
    	const addr& get_addr() const override;
    private:
        mutable std::string overflow_;
        mutable std::mutex state_mtx_;

        std::unique_ptr<basic_sync_sock> underlying_sock_;

        mode ssl_mode_;
        version version_;
        verification verification_;

        std::string cert_path_;
        std::string key_path_;
    	std::string ca_path_;

        WOLFSSL_CTX* ctx_ = nullptr;
        WOLFSSL* ssl_ = nullptr;

        bool handshake_complete_ = false;
        mutable bool read_eof_ = false;
        bool transport_eof_ = false;

        static void init_wolfssl_once();

        void create_ssl_context();
        void create_ssl_object();

        void ensure_ready() const;

        recv_result recv_internal(int timeout,
                                  const std::string* match,
                                  size_t eof) const;

        static void throw_ssl_error(const std::string& msg);
    };
}

#endif