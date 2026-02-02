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

#include <netkit/export.hpp>
#include <netkit/sock/basic_sync_sock.hpp>
#include <netkit/sock/addr.hpp>

#ifdef NETKIT_OPENSSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#include <memory>
#include <mutex>

namespace netkit::sock {
#ifdef NETKIT_OPENSSL
	enum class mode {
		client,
		server
	};

	enum class version {
		SSL_2 = SSL2_VERSION,
		SSL_3 = SSL3_VERSION,
		TLS_1_1 = TLS1_1_VERSION,
		TLS_1_2 = TLS1_2_VERSION,
		TLS_1_3 = TLS1_3_VERSION,
	};

	enum class verification {
		peer = SSL_VERIFY_PEER,
		none = SSL_VERIFY_NONE,
	};

    class NETKIT_API ssl_sync_sock {
    public:
        explicit ssl_sync_sock(std::unique_ptr<sock::basic_sync_sock> underlying,
                               mode ssl_mode, version ssl_version = version::TLS_1_2,
                               verification ssl_verification = verification::peer,
                               std::string cert_path = "",
                               std::string key_path = "");
        ~ssl_sync_sock();
        void connect() const;
        void bind() const;
        void unbind() const;
        void listen(int backlog) const;
        void listen() const;
        bool is_secure() const;
        std::unique_ptr<ssl_sync_sock> accept();
        int send(const void* buf, size_t len) const;
        void send(const std::string& buf) const;
        recv_result recv(int timeout_seconds) const;
        recv_result recv(int timeout_seconds, const std::string& match) const;
        recv_result recv(int timeout_seconds, const std::string& match, size_t eof) const;
        recv_result recv(int timeout_seconds, size_t eof) const;
        std::string overflow_bytes() const;
        void clear_overflow_bytes() const;
        void close();
        void perform_handshake();
    	[[nodiscard]] netkit::sock::addr get_peer() const;
    private:
        mutable std::string overflow_;
        mutable std::mutex state_mtx_;

        std::unique_ptr<basic_sync_sock> underlying_sock_;
        mode ssl_mode_;
        version version_;
        verification verification_;
        std::string cert_path_;
        std::string key_path_;

        SSL_CTX* ctx_ = nullptr;
        SSL* ssl_ = nullptr;

        BIO* read_bio_ = nullptr;
        BIO* write_bio_ = nullptr;

        bool handshake_complete_ = false;
        bool read_eof_ = false;
        bool transport_eof_ = false;

        static void init_openssl_once();
        void create_ssl_context();
        void create_ssl_object();
        void create_bio();
        void drain_write_bio() const;
        void feed_read_bio_blocking() const;
        void ensure_ready() const;
        recv_result recv_internal(int, const std::string* match, size_t eof) const;
        static void throw_ssl_error(const std::string& msg);
    };
#endif
}