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
#include <netkit/crypto/windows/certs.hpp>
#include <netkit/except.hpp>
#include <netkit/sock/sync_sock.hpp>
#include <netkit/sock/wolfssl/ssl_sync_sock.hpp>
#include <thread>

#ifdef NETKIT_WINDOWS
#include <netkit/utility.hpp>
#endif

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
	init_wolfssl_once();
	create_ssl_context();
	create_ssl_object();
}

netkit::sock::ssl_sync_sock::~ssl_sync_sock() {
	close();
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
	return ssl_ && handshake_complete_;
}

std::unique_ptr<netkit::sock::ssl_sync_sock> netkit::sock::ssl_sync_sock::accept() {
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

int netkit::sock::ssl_sync_sock::send(const void* buf, size_t len) const {
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

void netkit::sock::ssl_sync_sock::send(const std::string& buf) const {
	static_cast<void>(send(buf.data(), buf.size()));
}

netkit::sock::recv_result netkit::sock::ssl_sync_sock::recv(int timeout_seconds) const {
    return recv_internal(timeout_seconds, nullptr, 0);
}

netkit::sock::recv_result netkit::sock::ssl_sync_sock::recv(int timeout_seconds, const std::string& match) const {
    return recv_internal(timeout_seconds, &match, 0);
}

netkit::sock::recv_result netkit::sock::ssl_sync_sock::recv(int timeout_seconds, const std::string& match, size_t eof) const {
    return recv_internal(timeout_seconds, &match, eof);
}

netkit::sock::recv_result netkit::sock::ssl_sync_sock::recv(int timeout_seconds, size_t eof) const {
    return recv_internal(timeout_seconds, nullptr, eof);
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

void netkit::sock::ssl_sync_sock::perform_handshake()
{
	int r = wolfSSL_set_fd(
		ssl_,
		underlying_sock_->native_handle()
	);

	if (r != SSL_SUCCESS)
		throw std::runtime_error("set_fd failed");

	int ret;

	if (ssl_mode_ == mode::client)
		ret = wolfSSL_connect(ssl_);
	else
		ret = wolfSSL_accept(ssl_);

	if (ret != SSL_SUCCESS)
	{
		int err = wolfSSL_get_error(ssl_, ret);

		std::cerr << "wolfSSL_connect/accept ret="
				  << ret
				  << " err="
				  << err
				  << "\n";

		throw_ssl_error("TLS handshake failed");
	}

	handshake_complete_ = true;
}

void netkit::sock::ssl_sync_sock::init_wolfssl_once() {
	static std::once_flag flag;
	std::call_once(flag, []() {
		wolfSSL_Init();
		wolfSSL_Debugging_ON();
	});
}

void netkit::sock::ssl_sync_sock::create_ssl_context() {
	WOLFSSL_METHOD* method =
		(ssl_mode_ == mode::client)
		? wolfTLS_client_method()
		: wolfTLS_server_method();

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
		? SSL_VERIFY_PEER
		: SSL_VERIFY_NONE;

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
	if (!wolfSSL_CTX_load_verify_locations(ctx_, path_.c_str(), nullptr)) {
		throw std::runtime_error{"failed to load certificate location"};
	}

	loaded_ca = true;
#endif

	if (!loaded_ca) {
		loaded_ca =
			wolfSSL_CTX_load_system_CA_certs(ctx_) == SSL_SUCCESS;
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
}

void netkit::sock::ssl_sync_sock::create_ssl_object() {
	ssl_ = wolfSSL_new(ctx_);
	if (!ssl_) {
		throw_ssl_error("Failed to create WOLFSSL object");
	}

	auto hostname = this->underlying_sock_->get_addr().get_hostname();
	wolfSSL_UseSNI(ssl_, WOLFSSL_SNI_HOST_NAME, hostname.c_str(), hostname.length());
}

void netkit::sock::ssl_sync_sock::ensure_ready() const {
	if (!ssl_) {
		throw std::runtime_error("SSL not initialized");
	}

	if (!handshake_complete_) {
		const_cast<ssl_sync_sock*>(this)->perform_handshake();
	}
}

netkit::sock::recv_result netkit::sock::ssl_sync_sock::recv_internal(
	int timeout,
	const std::string* match,
	size_t eof) const
{
	std::lock_guard lock(state_mtx_);
	ensure_ready();

	auto start = std::chrono::steady_clock::now();

	char buffer[4096];

	while (true) {
		int ret = wolfSSL_read(ssl_, buffer, sizeof(buffer));

		if (ret > 0) {
			overflow_.append(buffer, ret);

			if (match) {
				auto pos = overflow_.find(*match);
				if (pos != std::string::npos) {
					std::string out = overflow_.substr(0, pos + match->size());
					overflow_.erase(0, pos + match->size());
					return {out, recv_status::success};
				}
			}

			if (eof && overflow_.size() >= eof) {
				std::string out = overflow_.substr(0, eof);
				overflow_.erase(0, eof);
				return {out, recv_status::success};
			}

			if (!match && eof == 0) {
				std::string out = overflow_;
				overflow_.clear();
				return {out, recv_status::success};
			}

			continue;
		}

		if (ret == 0) {
			read_eof_ = true;
			return {"", recv_status::closed};
		}

		int err = wolfSSL_get_error(ssl_, ret);

		if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {

			if (timeout > 0) {
				auto now = std::chrono::steady_clock::now();
				auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start);

				if (elapsed.count() >= timeout) {
					return {"", recv_status::timeout};
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}

		return {"", recv_status::error};
	}
}

void netkit::sock::ssl_sync_sock::throw_ssl_error(const std::string& msg) {
	int err = wolfSSL_get_error(nullptr, 0);
	throw std::runtime_error(msg + " (wolfSSL err=" + std::to_string(err) + ")");
}

#endif