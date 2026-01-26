/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file ssl_sync_sock_openssl.cpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Implementation of the synchronous SSL/TLS socket class using OpenSSL.
 */
#ifdef NETKIT_OPENSSL

#include <netkit/except.hpp>
#include <netkit/sock/openssl/ssl_sync_sock.hpp>
#include <netkit/sock/sync_sock.hpp>
#include <netkit/crypto/windows/certs.hpp>

#ifdef NETKIT_WINDOWS
#include <netkit/utility.hpp>
#endif

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>

#include <memory>
#include <mutex>

netkit::sock::ssl_sync_sock::ssl_sync_sock(std::unique_ptr<basic_sync_sock> underlying,
                       mode ssl_mode, version ssl_version,
                       verification ssl_verification,
                       std::string cert_path,
                       std::string key_path)
    : underlying_sock_(std::move(underlying)),
      ssl_mode_(ssl_mode),
      version_(ssl_version),
      verification_(ssl_verification),
      cert_path_(std::move(cert_path)), key_path_(std::move(key_path))
{
    init_openssl_once();
    create_ssl_context();
    create_ssl_object();
    create_bio();

    if (ssl_mode_ == mode::client) {
        auto underlying_hostname = underlying_sock_->get_addr().get_hostname();
        if (underlying_hostname.empty()) {
            throw std::runtime_error{"empty hostname"};
        }
        SSL_set_tlsext_host_name(ssl_, underlying_hostname.c_str());
    }
}

netkit::sock::ssl_sync_sock::~ssl_sync_sock() {
    this->close();
}

void netkit::sock::ssl_sync_sock::connect() const {
    if (ssl_mode_ != mode::client)
        throw std::runtime_error("connect() only valid for client mode");

    underlying_sock_->connect();
}

void netkit::sock::ssl_sync_sock::bind() const {
    underlying_sock_->bind();
}

void netkit::sock::ssl_sync_sock::unbind() const {
    underlying_sock_->unbind();
}

void netkit::sock::ssl_sync_sock::listen(int backlog) const {
    underlying_sock_->listen(backlog);
}

void netkit::sock::ssl_sync_sock::listen() const {
    underlying_sock_->listen();
}

bool netkit::sock::ssl_sync_sock::is_secure() const {
    return ssl_ && SSL_is_init_finished(ssl_);
}

std::unique_ptr<netkit::sock::ssl_sync_sock> netkit::sock::ssl_sync_sock::accept() {
    if (ssl_mode_ != mode::server)
        throw std::runtime_error("accept() only valid for server mode");
    auto new_sock = underlying_sock_->accept();
    if (!new_sock) return nullptr;

    auto child = std::make_unique<ssl_sync_sock>(std::move(new_sock),
                                                 mode::server,
                                                 version_,
                                                 verification_,
                                                 cert_path_, key_path_);
    return child;
}

int netkit::sock::ssl_sync_sock::send(const void* buf, size_t len) const {
    ensure_ready();

    size_t offset = 0;
    while (offset < len) {
        int ret = SSL_write(
            ssl_,
            static_cast<const char*>(buf) + offset,
            static_cast<int>(len - offset));

        drain_write_bio();

        if (ret > 0) {
            offset += ret;
            continue;
        }

        int err = SSL_get_error(ssl_, ret);
        if (err == SSL_ERROR_WANT_READ) {
            feed_read_bio_blocking();
        } else if (err == SSL_ERROR_WANT_WRITE) {
            // retry
        } else {
            throw_ssl_error("SSL_write failed");
        }
    }

    return static_cast<int>(len);
}

void netkit::sock::ssl_sync_sock::send(const std::string& buf) const {
    static_cast<void>(send(buf.data(), buf.size()));
}

netkit::sock::sock_recv_result netkit::sock::ssl_sync_sock::recv(int timeout_seconds) const {
    return recv_internal(timeout_seconds, nullptr, 0);
}

netkit::sock::sock_recv_result netkit::sock::ssl_sync_sock::recv(int timeout_seconds, const std::string& match) const {
    return recv_internal(timeout_seconds, &match, 0);
}

netkit::sock::sock_recv_result netkit::sock::ssl_sync_sock::recv(int timeout_seconds, const std::string& match, size_t eof) const {
    return recv_internal(timeout_seconds, &match, eof);
}

netkit::sock::sock_recv_result netkit::sock::ssl_sync_sock::recv(int timeout_seconds, size_t eof) const {
    return recv_internal(timeout_seconds, nullptr, eof);
}

std::string netkit::sock::ssl_sync_sock::overflow_bytes() const {
    return overflow_;
}

void netkit::sock::ssl_sync_sock::clear_overflow_bytes() const {
    overflow_.clear();
}

void netkit::sock::ssl_sync_sock::close() {
    std::scoped_lock lk(state_mtx_);

    if (ssl_) {
        SSL_shutdown(ssl_); // ignore result for sync wrapper
        SSL_free(ssl_);
        ssl_ = nullptr;
    }

    if (ctx_) {
        SSL_CTX_free(ctx_);
        ctx_ = nullptr;
    }

    if (underlying_sock_) { // should not be required but just in case
        underlying_sock_->close();
    }
}

void netkit::sock::ssl_sync_sock::perform_handshake() {
    while (!SSL_is_init_finished(ssl_)) {
        int ret = SSL_do_handshake(ssl_);
        drain_write_bio();

        if (ret == 1)
            break;

        int err = SSL_get_error(ssl_, ret);
        if (err == SSL_ERROR_WANT_READ) {
            feed_read_bio_blocking();
        } else if (err == SSL_ERROR_WANT_WRITE) {
            continue;
        } else {
            throw_ssl_error("TLS handshake failed");
        }
    }

    handshake_complete_ = true;
}

void netkit::sock::ssl_sync_sock::init_openssl_once() {
    static bool initialized = false;
    static std::mutex m;
    std::scoped_lock lk(m);
    if (!initialized) {
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
        initialized = true;
    }
}

void netkit::sock::ssl_sync_sock::create_ssl_context() {
    const SSL_METHOD* method = (ssl_mode_ == mode::client)
                                ? TLS_client_method()
                                : TLS_server_method();
    ctx_ = SSL_CTX_new(method);
    if (!ctx_) throw_ssl_error("SSL_CTX_new failed");

    SSL_CTX_set_min_proto_version(ctx_, static_cast<long>(version_));

    if (ssl_mode_ == mode::server) {
        if (SSL_CTX_use_certificate_file(ctx_, cert_path_.c_str(), SSL_FILETYPE_PEM) <= 0)
            throw_ssl_error("Failed to load certificate");
        if (SSL_CTX_use_PrivateKey_file(ctx_, key_path_.c_str(), SSL_FILETYPE_PEM) <= 0)
            throw_ssl_error("Failed to load private key");
    } else {
        SSL_CTX_set_verify(ctx_, static_cast<int>(verification_), nullptr);
        SSL_CTX_set_verify_depth(ctx_, 10);
    	SSL_CTX_set_default_verify_paths(ctx_);

		if (const char* ca_path = std::getenv("SSL_CERT_FILE")) {
    		if (!SSL_CTX_load_verify_locations(ctx_, ca_path, nullptr)) {
    			throw std::runtime_error{"failed to load ca bundle from environment variable (SSL_CERT_FILE=" + std::string(ca_path) + ")"};
    		}
    	}

        if (!cert_path_.empty()) {
		    if (SSL_CTX_load_verify_locations(ctx_, cert_path_.c_str(), nullptr) != 1) {
		        BIO* bio = BIO_new_mem_buf(cert_path_.data(), static_cast<int>(cert_path_.size()));
		        if (!bio) throw std::runtime_error("failed to create BIO");

		        while (true) {
		            X509* cert = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr);
		            if (!cert) break;
		            if (X509_STORE_add_cert(SSL_CTX_get_cert_store(ctx_), cert) != 1) {
		                X509_free(cert);
		                BIO_free(bio);
		                throw std::runtime_error("failed to add certificate to store");
		            }
		            X509_free(cert);
		        }
		        BIO_free(bio);
		    }
		}

        const auto has_usable_certs = [](const SSL_CTX* ctx) -> bool {
            if (!ctx) return false;

            X509_STORE* store = SSL_CTX_get_cert_store(ctx);
            if (!store) return false;

            STACK_OF(X509_OBJECT)* objs = X509_STORE_get0_objects(store);
            if (!objs) return false;

            for (int i = 0; i < sk_X509_OBJECT_num(objs); ++i) {
                X509_OBJECT* obj = sk_X509_OBJECT_value(objs, i);
                if (!obj) continue;

                if (X509_OBJECT_get_type(obj) == X509_LU_X509) {
                    return true;
                }
            }

            return false;
		};
#ifdef NETKIT_WINDOWS
        const auto get_localappdata = []() -> std::filesystem::path {
            const std::string folder_name = "netkit";

            std::filesystem::path base_path;

            char appdata[MAX_PATH];
            DWORD len = GetEnvironmentVariableA("LOCALAPPDATA", appdata, sizeof(appdata));
            if (len > 0) {
                base_path = appdata;
            } else {
                base_path = std::filesystem::temp_directory_path();
            }
            base_path /= folder_name;

            std::filesystem::create_directories(base_path);
            return base_path;
        };

        std::filesystem::path path = (get_localappdata() / "ca-bundle.pem").string();
        if (!has_usable_certs(ctx_) && crypto::windows::is_outdated(path.wstring())) {
            std::filesystem::remove(path);
            if (!crypto::windows::export_certs(path.wstring())) {
                throw std::runtime_error("failed to export certificates");
            }
        }

        const std::string path_ = path.string();
        if (!SSL_CTX_load_verify_locations(ctx_, path_.c_str(), nullptr)) {
            throw std::runtime_error{"failed to load certificate location"};
        }
#endif

        X509_VERIFY_PARAM_set1_host(SSL_CTX_get0_param(ctx_),
                                       underlying_sock_->get_addr().get_hostname().c_str(),
                                       0);
    }
}

void netkit::sock::ssl_sync_sock::create_ssl_object() {
    ssl_ = SSL_new(ctx_);
    if (!ssl_) throw_ssl_error("SSL_new failed");
}

void netkit::sock::ssl_sync_sock::create_bio() {
    read_bio_ = BIO_new(BIO_s_mem());
    BIO_set_mem_eof_return(read_bio_, -1);
    write_bio_ = BIO_new(BIO_s_mem());
    if (!read_bio_ || !write_bio_)
        throw_ssl_error("Failed to create memory BIOs");

    SSL_set_bio(ssl_, read_bio_, write_bio_);

    if (ssl_mode_ == mode::client) {
        SSL_set_connect_state(ssl_);
    } else {
        SSL_set_accept_state(ssl_);
    }
}

void netkit::sock::ssl_sync_sock::drain_write_bio() const {
    char buf[4096];
    int n;

    while ((n = BIO_read(write_bio_, buf, sizeof(buf))) > 0) {
        underlying_sock_->send(buf, n);
    }
}

void netkit::sock::ssl_sync_sock::feed_read_bio_blocking() const {
    auto res = underlying_sock_->primitive_recv();

    if (res.status == sock::sock_recv_status::closed) {
        if (!handshake_complete_) {
            throw std::runtime_error("Socket closed during TLS handshake");
        }

        BIO_set_mem_eof_return(read_bio_, 0);
        return;
    }

    if (res.status != sock::sock_recv_status::success)
        throw std::runtime_error("Socket read failed");

    if (!res.data.empty()) {
        int written = BIO_write(
            read_bio_,
            res.data.data(),
            static_cast<int>(res.data.size()));
        if (written <= 0)
            throw_ssl_error("BIO_write failed");
    }
}

void netkit::sock::ssl_sync_sock::ensure_ready() const {
    if (!ssl_) throw std::runtime_error("SSL socket closed");
}

netkit::sock::sock_recv_result netkit::sock::ssl_sync_sock::recv_internal(int, const std::string* match, size_t eof) const {
    ensure_ready();
    sock::sock_recv_result result;

    if (!overflow_.empty()) {
        result.data = std::exchange(overflow_, "");
    }

    while (true) {
        char buf[4096];
        int ret = SSL_read(ssl_, buf, sizeof(buf));
        drain_write_bio();

        if (ret > 0) {
            result.data.append(buf, ret);
        } else {
            int err = SSL_get_error(ssl_, ret);
            if (err == SSL_ERROR_WANT_READ) {
                const_cast<ssl_sync_sock*>(this)->feed_read_bio_blocking();
                continue;
            } else if (err == SSL_ERROR_WANT_WRITE) {
                continue;
            } else if (err == SSL_ERROR_ZERO_RETURN) {
                result.status = sock::sock_recv_status::closed;
                break;
            } else {
                result.status = sock::sock_recv_status::error;
                break;
            }
        }

        if (match) {
            auto pos = result.data.find(*match);
            if (pos != std::string::npos) {
                overflow_ = result.data.substr(pos + match->size());
                result.data.resize(pos + match->size());
                break;
            }
        }

        if (eof && result.data.size() >= eof) {
            overflow_ = result.data.substr(eof);
            result.data.resize(eof);
            break;
        }
    }

    return result;
}

void netkit::sock::ssl_sync_sock::throw_ssl_error(const std::string& msg) {
    char buf[256];
    ERR_error_string_n(ERR_get_error(), buf, sizeof(buf));
    throw std::runtime_error(msg + ": " + buf);
}
#endif