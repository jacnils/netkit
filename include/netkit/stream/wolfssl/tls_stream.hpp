#pragma once

#ifdef NETKIT_WOLFSSL

#include <netkit/tcp/tcp_stream.hpp>
#include <netkit/stream/tls_stream_enum.hpp>
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>

namespace netkit::stream {
class tls_stream : public netkit::stream::basic_stream {
public:
	explicit tls_stream(std::unique_ptr<tcp::tcp_stream> stream,
		version ver = version::TLS_1_2, verification verif = verification::peer,
		const std::string& ca_cert = {});

	~tls_stream() override;

	void perform_handshake() const;

	netkit::stream::stream_result read(std::span<std::byte> buffer) override;
	netkit::stream::stream_result write(std::span<const std::byte> buffer) override;

	void close() noexcept override;

private:
	std::unique_ptr<tcp::tcp_stream> stream_;
	version version_;
	verification verification_;

	std::string ca_cert_;

	WOLFSSL_CTX* ctx_{};
	WOLFSSL* ssl_{};
};

}

#endif