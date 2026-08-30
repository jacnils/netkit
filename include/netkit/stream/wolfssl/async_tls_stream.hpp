#pragma once

#ifdef NETKIT_WOLFSSL

#include <netkit/tcp/async_tcp_stream.hpp>
#include <netkit/stream/tls_stream_enum.hpp>
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>

namespace netkit::stream {
class async_tls_stream : public netkit::stream::basic_async_stream {
public:
	using basic_async_stream::write;
	using basic_async_stream::read;

	explicit async_tls_stream(std::unique_ptr<tcp::async_tcp_stream> stream,
		version ver = version::TLS_1_2, verification verif = verification::peer,
		std::string  ca_cert = {}, const std::string& sni = {});

	~async_tls_stream() override;

	netkit::io::task<void> perform_handshake();

	netkit::io::task<netkit::stream::stream_result> read(std::span<std::byte> buffer) override;
	netkit::io::task<netkit::stream::stream_result> write(std::span<const std::byte> buffer) override;

	void close() noexcept override;

	[[nodiscard]] bool is_open() const noexcept override;

private:
	std::unique_ptr<tcp::async_tcp_stream> stream_;
	version version_;
	verification verification_;

	std::string ca_cert_;

	WOLFSSL_CTX* ctx_{};
	WOLFSSL* ssl_{};
};

}

#endif