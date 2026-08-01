#ifdef NETKIT_WOLFSSL

#include <filesystem>
#include <memory>
#include <mutex>

#ifdef NETKIT_WOLFSSL_DEBUG
#include <iostream>
#endif

#include <netkit/crypto/windows/certs.hpp>
#include <netkit/crypto/fallback_ca.hpp>
#include <netkit/stream/wolfssl/tls_stream.hpp>

netkit::stream::tls_stream::tls_stream(std::unique_ptr<tcp::tcp_stream> stream, version ver, verification verif, const std::string& ca_cert)
: stream_(std::move(stream)), version_(ver), verification_(verif), ca_cert_(ca_cert) {
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

	ctx_ = wolfSSL_CTX_new(wolfTLS_client_method());

	if (!ctx_)
		throw std::runtime_error("wolfSSL_CTX_new failed");

	// TODO: maybe we shouldn't store the version in the class? haven't yet decided on this
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
	}

	int verify_mode = (verification_ == verification::peer) ? WOLFSSL_VERIFY_PEER : WOLFSSL_VERIFY_NONE;
	wolfSSL_CTX_set_verify(ctx_, verify_mode, nullptr);

	bool loaded_ca = false;
	if (this->ca_cert_.empty()) {
		loaded_ca = wolfSSL_CTX_load_verify_buffer(ctx_,
			reinterpret_cast<const unsigned char*>(crypto::fallback_ca.data()),
			static_cast<long>(crypto::fallback_ca.size()), WOLFSSL_FILETYPE_PEM);
	}
#ifndef NETKIT_DKP
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
	if (!loaded_ca && wolfSSL_CTX_load_verify_locations(ctx_, path_.c_str(), nullptr)) {
		loaded_ca = true;
	}

#endif
#endif
	if (!loaded_ca) {
		loaded_ca = wolfSSL_CTX_load_system_CA_certs(ctx_) == SSL_SUCCESS;
	}
#endif

#ifdef NETKIT_ENABLE_FALLBACK_CA
	if (!loaded_ca) {
		loaded_ca = wolfSSL_CTX_load_verify_buffer(
			ctx_,
			reinterpret_cast<const unsigned char*>(crypto::fallback_ca.data()),
			crypto::fallback_ca.size(),
			WOLFSSL_FILETYPE_PEM
		);
	}
#endif

	if (!loaded_ca && verification_ == verification::peer) {
		throw std::runtime_error(
			"No trusted CA certificates available"
		);
	}

	wolfSSL_SetIORecv(ctx_,
		[](WOLFSSL* ssl, char* buf, int sz, void* ctx) -> int {
			auto* self = static_cast<tls_stream*>(ctx);

			auto res = self->stream_->read(std::span<std::byte>(reinterpret_cast<std::byte*>(buf), sz) );

			if (res.status != netkit::stream::stream_status::success)
				return WOLFSSL_CBIO_ERR_GENERAL;

			if (res.bytes == 0)
				return WOLFSSL_CBIO_ERR_CONN_CLOSE;

			return static_cast<int>(res.bytes);
		}
	);

	wolfSSL_SetIOSend(ctx_,
		[](WOLFSSL* ssl, char* buf, int sz, void* ctx) -> int {
			auto* self = static_cast<tls_stream*>(ctx);

			auto res = self->stream_->write(std::span<const std::byte>(reinterpret_cast<std::byte*>(buf), sz) );

			if (res.status != netkit::stream::stream_status::success)
				return WOLFSSL_CBIO_ERR_GENERAL;

			return static_cast<int>(res.bytes);
		}
	);

	ssl_ = wolfSSL_new(ctx_);

	if (!ssl_)
		throw std::runtime_error("wolfSSL_new failed");

	wolfSSL_SetIOReadCtx(ssl_, this);
	wolfSSL_SetIOWriteCtx(ssl_, this);

	std::string hostname;
	if (this->stream_ && this->stream_->get_addr().has_value()) {
		hostname = this->stream_->get_addr()->get_hostname();
		wolfSSL_UseSNI(ssl_, WOLFSSL_SNI_HOST_NAME, hostname.data(), hostname.length());
		wolfSSL_check_domain_name(ssl_, hostname.c_str());
	}
}

void netkit::stream::tls_stream::perform_handshake() const {
	int ret = wolfSSL_connect(ssl_);

	if (ret != WOLFSSL_SUCCESS) {
		int err = wolfSSL_get_error(ssl_, ret);

		throw std::runtime_error("TLS handshake failed: " + std::to_string(err));
	}
}

netkit::stream::stream_result netkit::stream::tls_stream::read(std::span<std::byte> buffer) {
	for (;;) {
		int ret = wolfSSL_read(ssl_, buffer.data(), static_cast<int>(buffer.size()));

		if (ret > 0)
			return { static_cast<size_t>(ret), stream_status::success };

		int err = wolfSSL_get_error(ssl_, ret);

		if (err == WOLFSSL_ERROR_ZERO_RETURN)
			return { 0, stream_status::eof };

		if (err == WOLFSSL_ERROR_WANT_READ || err == WOLFSSL_ERROR_WANT_WRITE)
			continue;

		return { 0, stream_status::eof };
	}
}

netkit::stream::stream_result netkit::stream::tls_stream::write(std::span<const std::byte> buffer) {
	int ret = wolfSSL_write(ssl_, buffer.data(), static_cast<int>(buffer.size()));

	if (ret > 0) {
		return { static_cast<std::size_t>(ret), stream_status::success };
	}

	return { 0, stream_status::error };
}

void netkit::stream::tls_stream::close() noexcept {
	if (ssl_)
		wolfSSL_shutdown(ssl_);

	stream_->close();
}

netkit::stream::tls_stream::~tls_stream() {
	if (ssl_)
		wolfSSL_free(ssl_);

	if (ctx_)
		wolfSSL_CTX_free(ctx_);
}

#endif