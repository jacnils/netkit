/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file ssl_sync_sock_wolfssl.cpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Implementation of the synchronous SSL/TLS socket class using WolfSSL.
 */
#ifdef NETKIT_WOLFSSL
#include <iostream>
#include <netkit/except.hpp>
#include <netkit/sock/sync_sock.hpp>
#include <netkit/sock/wolfssl/ssl_sync_sock.hpp>
#include <thread>

#ifdef NETKIT_ENABLE_FALLBACK_CA
#include <netkit/crypto/fallback_ca.hpp>
#endif

#ifdef NETKIT_WINDOWS
#include <netkit/utility.hpp>
#include <netkit/crypto/windows/certs.hpp>
#endif

#include <memory>
#include <mutex>

template<typename T>
std::unique_ptr<T> unique_dynamic_cast(std::unique_ptr<netkit::sock::basic_sync_sock> base)
{
	if (auto ptr = dynamic_cast<T*>(base.get())) {
		base.release();
		return std::unique_ptr<T>(ptr);
	}
	return nullptr;
}

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
	underlying_sock_->set_sock_opts(opt::no_blocking);

	init_wolfssl_once();
	create_ssl_context();
	create_ssl_object();
}

netkit::sock::ssl_sync_sock::~ssl_sync_sock() {
	ssl_sync_sock::close();
}

void netkit::sock::ssl_sync_sock::connect() {
    if (ssl_mode_ != mode::client)
        throw std::runtime_error("connect() only valid for client mode");

    underlying_sock_->connect();
}

void netkit::sock::ssl_sync_sock::bind() {
    underlying_sock_->bind();
}

void netkit::sock::ssl_sync_sock::unbind() {
    underlying_sock_->unbind();
}

void netkit::sock::ssl_sync_sock::listen(int backlog) {
    underlying_sock_->listen(backlog);
}

void netkit::sock::ssl_sync_sock::listen() {
    underlying_sock_->listen();
}

bool netkit::sock::ssl_sync_sock::is_secure() const {
	return ssl_ && handshake_complete_;
}

std::unique_ptr<netkit::sock::basic_sync_sock> netkit::sock::ssl_sync_sock::accept() {
	auto client = underlying_sock_->accept();

	auto ssl_client = std::make_unique<ssl_sync_sock>(
		std::move(client),
		mode::server,
		version_,
		verification_,
		cert_path_,
		key_path_);

	ssl_client->perform_handshake();

	return ssl_client;
}
std::unique_ptr<netkit::sock::ssl_sync_sock> netkit::sock::ssl_sync_sock::accept_explicit_ssl() {
	auto accepted = accept();
	return unique_dynamic_cast<ssl_sync_sock>(std::move(accepted));
}

int netkit::sock::ssl_sync_sock::send(const void* buf, size_t len) {
	ensure_ready();

	int ret = wolfSSL_write(
		ssl_,
		buf,
		static_cast<int>(len)
	);

	if (ret <= 0) {
		int err = wolfSSL_get_error(ssl_, ret);
		throw std::runtime_error(
			"wolfSSL_write failed: " + std::to_string(err)
		);
	}

	return ret;
}

void netkit::sock::ssl_sync_sock::send(const std::string& buf) {
	static_cast<void>(send(buf.data(), buf.size()));
}

netkit::sock::recv_result netkit::sock::ssl_sync_sock::recv(int timeout_seconds) {
    return recv_internal(timeout_seconds, nullptr, 0);
}

netkit::sock::recv_result netkit::sock::ssl_sync_sock::recv(int timeout_seconds, const std::string& match) {
    return recv_internal(timeout_seconds, &match, 0);
}

netkit::sock::recv_result netkit::sock::ssl_sync_sock::recv(int timeout_seconds, const std::string& match, size_t eof) {
    return recv_internal(timeout_seconds, &match, eof);
}

netkit::sock::recv_result netkit::sock::ssl_sync_sock::recv(int timeout_seconds, size_t eof) {
    return recv_internal(timeout_seconds, nullptr, eof);
}

netkit::sock::recv_result netkit::sock::ssl_sync_sock::recv() {
	for (;;) {
		char buf[8192];

		int n = wolfSSL_read(ssl_, buf, sizeof(buf));

		if (n > 0) {
			return {{buf, buf + n}, recv_status::success};
		}

		switch (int err = wolfSSL_get_error(ssl_, n)) {
		case WOLFSSL_ERROR_ZERO_RETURN:
			return {{}, recv_status::closed};

		case WOLFSSL_ERROR_WANT_READ:
		case WOLFSSL_ERROR_WANT_WRITE:
			continue; // or wait for socket readiness

		default:
			throw netkit::socket_error(
				"wolfSSL_read failed: " + std::to_string(err)
			);
		}
	}
}

std::string netkit::sock::ssl_sync_sock::overflow_bytes() const {
    return overflow_;
}

void netkit::sock::ssl_sync_sock::clear_overflow_bytes() const {
    overflow_.clear();
}

netkit::sock::addr netkit::sock::ssl_sync_sock::get_peer() const {
	return underlying_sock_->get_peer();
}
netkit::sock::addr& netkit::sock::ssl_sync_sock::get_addr() {
	return underlying_sock_->get_addr();
}
const netkit::sock::addr& netkit::sock::ssl_sync_sock::get_addr() const {
	return underlying_sock_->get_addr();
}

void netkit::sock::ssl_sync_sock::close() {
	std::lock_guard lock(state_mtx_);

	if (ssl_) {
		wolfSSL_shutdown(ssl_);
		wolfSSL_free(ssl_);
		ssl_ = nullptr;
	}

	if (ctx_) {
		wolfSSL_CTX_free(ctx_);
		ctx_ = nullptr;
	}

	if (underlying_sock_) {
		underlying_sock_->close();
	}
}

#ifdef NETKIT_DKP
void netkit::sock::ssl_sync_sock::perform_handshake() {
	int ret;

	if (ssl_mode_ == mode::client) {
		ret = wolfSSL_connect(ssl_);
	} else
		ret = wolfSSL_accept(ssl_);

	if (ret != WOLFSSL_SUCCESS) {
#ifdef NETKIT_WOLFSSL_DEBUG
		int err = wolfSSL_get_error(ssl_, ret);

		std::cerr << "wolfSSL_connect/accept ret="
				  << ret
				  << " err="
				  << err
				  << "\n";
#endif

		throw_ssl_error("TLS handshake failed");
	}

	handshake_complete_ = true;
}
#else
void netkit::sock::ssl_sync_sock::perform_handshake() {
	while (true) {
		int ret = ssl_mode_ == mode::client
			? wolfSSL_connect(ssl_)
			: wolfSSL_accept(ssl_);

		if (ret == WOLFSSL_SUCCESS) {
			handshake_complete_ = true;
			return;
		}

		int err = wolfSSL_get_error(ssl_, ret);

		if (err == WOLFSSL_ERROR_WANT_READ ||
			err == WOLFSSL_ERROR_WANT_WRITE) {
			std::this_thread::yield();
			continue;
			}

		throw_ssl_error("TLS handshake failed");
	}
}
#endif

void netkit::sock::ssl_sync_sock::init_wolfssl_once() {
	static std::once_flag flag;
	std::call_once(flag, []() {
		wolfSSL_Init();
#if defined(NETKIT_WOLFSSL_DEBUG)
		wolfSSL_Debugging_ON();
		wolfSSL_SetLoggingCb([](const int level, const char* msg) {
#ifdef NETKIT_DKP
			SYS_Report("[wolfSSL:%d] %s\n", level, msg);
#else
			std::cerr << msg << "\n";
#endif
		});
		wolfSSL_SetAllocators(
			[](size_t sz) -> void* {
				void* p = malloc(sz);
				printf("malloc(%zu) = %p\n", sz, p);
				return p;
			},
			[](void* p) {
				printf("free(%p)\n", p);
				free(p);
			},
			[](void* p, size_t sz) -> void* {
				void* np = realloc(p, sz);
				printf("realloc(%p, %zu) = %p\n", p, sz, np);
				return np;
			}
		);
#endif
	});
}

void netkit::sock::ssl_sync_sock::create_ssl_context() {
	WOLFSSL_METHOD* method =
		(ssl_mode_ == mode::client)
		? wolfTLS_client_method()
		: wolfTLS_server_method();

	if (!method) {
		throw_ssl_error("wolfTLS method initialization failed");
	}

	ctx_ = wolfSSL_CTX_new(method);
	if (!ctx_) {
		throw_ssl_error("wolfSSL_CTX_new failed");
	}

	switch (version_) {
	case version::TLS_1_2:
		wolfSSL_CTX_SetMinVersion(ctx_, WOLFSSL_TLSV1_2);
		break;

	case version::TLS_1_3:
		wolfSSL_CTX_SetMinVersion(ctx_, WOLFSSL_TLSV1_3);
		break;

	case version::TLS_1_1:
		wolfSSL_CTX_SetMinVersion(ctx_, WOLFSSL_TLSV1_1);
		break;

	case version::TLS_1_0:
		wolfSSL_CTX_SetMinVersion(ctx_, WOLFSSL_TLSV1);
		break;
	}

	if (const char* env = std::getenv("NETKIT_SSL_VERIFY")) {
		std::string value(env);

		if (value == "none" || value == "disable" || value == "false") {
			verification_ = verification::none;
		}

		if (value == "peer" || value == "verify" || value == "enable" || value == "true") {
			verification_ = verification::peer;
		}
	}

	int verify_mode = (verification_ == verification::peer)
		? WOLFSSL_VERIFY_PEER
		: WOLFSSL_VERIFY_NONE;

	wolfSSL_CTX_set_verify(ctx_, verify_mode, nullptr);

	bool loaded_ca = false;

#ifndef NETKIT_DKP
	if (!ca_path_.empty()) {
		loaded_ca = wolfSSL_CTX_load_verify_locations(
			ctx_,
			ca_path_.c_str(),
			nullptr
		) == SSL_SUCCESS;
	}

#ifdef NETKIT_WINDOWS
#ifdef NETKIT_ENABLE_WINDOWS_CERTSTORE
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
	if (!loaded_ca && crypto::windows::is_outdated(path.wstring())) {
		std::filesystem::remove(path);
		if (!crypto::windows::export_certs(path.wstring())) {
			throw std::runtime_error("failed to export certificates");
		}
	}

	const std::string path_ = path.string();
	if (wolfSSL_CTX_load_verify_locations(ctx_, path_.c_str(), nullptr)) {
		loaded_ca = true;
	}

#endif
#endif

	if (!loaded_ca && this->ssl_mode_ == mode::client) {
		loaded_ca =
			wolfSSL_CTX_load_system_CA_certs(ctx_) == SSL_SUCCESS;
	}
#endif

#ifdef NETKIT_ENABLE_FALLBACK_CA
	if (!loaded_ca && this->ssl_mode_ == mode::client) {
		loaded_ca = wolfSSL_CTX_load_verify_buffer(
			ctx_,
			reinterpret_cast<const unsigned char*>(crypto::fallback_ca.data()),
			static_cast<long>(crypto::fallback_ca.size()),
			WOLFSSL_FILETYPE_PEM
		);
	}
#endif

	if (!loaded_ca && verification_ == verification::peer && this->ssl_mode_ == mode::client) {
		throw std::runtime_error(
			"No trusted CA certificates available"
		);
	}

#ifndef NETKIT_DKP
	if (!cert_path_.empty() && this->ssl_mode_ == mode::server) {
		if (wolfSSL_CTX_use_certificate_file(ctx_, cert_path_.c_str(), SSL_FILETYPE_PEM) != SSL_SUCCESS)
			throw_ssl_error("Failed to load cert");
	}

	if (!key_path_.empty() && this->ssl_mode_ == mode::server) {
		if (wolfSSL_CTX_use_PrivateKey_file(ctx_, key_path_.c_str(), SSL_FILETYPE_PEM) != SSL_SUCCESS)
			throw_ssl_error("Failed to load key");
	}
#endif

#ifdef NETKIT_DKP
	wolfSSL_CTX_SetIOSend(ctx_, [](WOLFSSL*, char* buf, int sz, void* ctx) -> int {
		auto* self = static_cast<netkit::sock::ssl_sync_sock*>(ctx);

		int ret = self->underlying_sock_->send(
			buf,
			static_cast<size_t>(sz)
		);

		if (ret < 0)
			return WOLFSSL_CBIO_ERR_GENERAL;

		return ret;
	});

	wolfSSL_CTX_SetIORecv(ctx_,
	[](WOLFSSL*, char* buf, int sz, void* ctx) -> int {
		auto* self = static_cast<ssl_sync_sock*>(ctx);

		ssize_t n = ::recv(
			self->underlying_sock_->native_handle(),
			buf,
			sz,
			MSG_DONTWAIT
		);

		if (n > 0)
			return static_cast<int>(n);

		if (n == 0)
			return WOLFSSL_CBIO_ERR_CONN_CLOSE;

		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return WOLFSSL_CBIO_ERR_WANT_READ;

		if (errno == EINTR)
			return WOLFSSL_CBIO_ERR_WANT_READ;

		return WOLFSSL_CBIO_ERR_GENERAL;
	});
#endif
}

void netkit::sock::ssl_sync_sock::create_ssl_object() {
	ssl_ = wolfSSL_new(ctx_);
	if (!ssl_) {
		throw_ssl_error("Failed to create WOLFSSL object");
	}
	auto hostname = this->underlying_sock_->get_addr().get_hostname();
	if (hostname.empty()) {
		throw std::runtime_error{"get_hostname() empty"};
	}

	wolfSSL_UseSNI(ssl_, WOLFSSL_SNI_HOST_NAME, hostname.data(), hostname.length());
#ifdef NETKIT_DKP
	wolfSSL_SetIOWriteCtx(ssl_, this);
	wolfSSL_SetIOReadCtx(ssl_, this);
#else
	wolfSSL_set_fd(ssl_, underlying_sock_->native_handle());
#endif

	wolfSSL_check_domain_name(
		ssl_,
		hostname.c_str()
	);
}

void netkit::sock::ssl_sync_sock::ensure_ready() const {
	if (!ssl_) {
		throw std::runtime_error("SSL not initialized");
	}

	if (!handshake_complete_) {
		const_cast<ssl_sync_sock*>(this)->perform_handshake();
	}
}

netkit::sock::recv_result netkit::sock::ssl_sync_sock::recv_internal(int timeout_seconds, const std::string* match, size_t eof) const
{
    std::lock_guard lock(state_mtx_);
    ensure_ready();

    std::string data = overflow_;
    overflow_.clear();

    auto start = std::chrono::steady_clock::now();

    char buf[8192];

    while (true) {
        if (timeout_seconds != -1) {
            auto elapsed = std::chrono::steady_clock::now() - start;
            if (elapsed >= std::chrono::seconds(timeout_seconds)) {
                return {data, recv_status::timeout};
            }
        }

        int ret = wolfSSL_read(
            ssl_,
            buf,
            sizeof(buf)
        );

        if (ret > 0) {
            data.append(buf, static_cast<size_t>(ret));

            if (eof != 0 && data.size() >= eof) {
                if (data.size() > eof) {
                    overflow_ = data.substr(eof);
                    data.resize(eof);
                }

                return {data, recv_status::success};
            }

            if (match && !match->empty()) {
                auto pos = data.find(*match);

                if (pos != std::string::npos) {
                    overflow_ = data.substr(pos + match->size());
                    data.resize(pos + match->size());

                    return {data, recv_status::success};
                }
            }

            continue;
        }


    	int err = wolfSSL_get_error(ssl_, ret);
    	if (err == WOLFSSL_ERROR_WANT_READ ||
			err == WOLFSSL_ERROR_WANT_WRITE)
    	{
    		std::this_thread::sleep_for(std::chrono::milliseconds(1));
    		continue;
    	}

    	if (err == WOLFSSL_ERROR_ZERO_RETURN ||
			err == WOLFSSL_ERROR_SYSCALL ||
			err == -397)
    	{
    		if (!data.empty())
    			return {data, recv_status::success};

    		return {"", recv_status::closed};
    	}

    	return {data, recv_status::error};
    }
}

void netkit::sock::ssl_sync_sock::throw_ssl_error(const std::string& msg) {
	int err = wolfSSL_get_error(nullptr, 0);

	char buffer[256];
	wolfSSL_ERR_error_string(err, buffer);

	throw std::runtime_error(msg + " (wolfSSL err=" + std::to_string(err) + ", " + buffer + ")");
}

#endif