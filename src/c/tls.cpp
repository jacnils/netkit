#ifdef NETKIT_WOLFSSL

#include "internal.hpp"

#include <netkit/c/tls.h>

#include <netkit/stream/wolfssl/tls_stream.hpp>
#include <netkit/tcp/tcp_stream.hpp>

using netkit::stream::tls_stream;
using netkit::tcp::tcp_stream;

namespace {
	inline tls_stream& as_tls(nk_tls_stream_t* t) { return *reinterpret_cast<tls_stream*>(t); }
	inline nk_tls_stream_t* to_c(tls_stream* t) { return reinterpret_cast<nk_tls_stream_t*>(t); }
} // namespace

extern "C" nk_status_t nk_tls_stream_create(nk_tcp_stream_t* stream, nk_tls_version_t version,
                                             nk_tls_verification_t verification, const char* ca_cert,
                                             const char* sni, nk_tls_stream_t** out) {
	return netkit::c::wrap_checked(stream != nullptr && out != nullptr, [&] {
		std::unique_ptr<tcp_stream> owned(reinterpret_cast<tcp_stream*>(stream));
		*out = to_c(new tls_stream(std::move(owned), static_cast<netkit::stream::version>(version),
		                            static_cast<netkit::stream::verification>(verification),
		                            ca_cert ? std::string(ca_cert) : std::string(), sni ? std::string(sni) : std::string()));
	});
}

extern "C" void nk_tls_stream_destroy(nk_tls_stream_t* t) { delete reinterpret_cast<tls_stream*>(t); }

extern "C" nk_status_t nk_tls_stream_handshake(nk_tls_stream_t* t) {
	return netkit::c::wrap_checked(t != nullptr, [&] { as_tls(t).perform_handshake(); });
}

extern "C" void nk_tls_stream_close(nk_tls_stream_t* t) {
	if (t) as_tls(t).close();
}

extern "C" bool nk_tls_stream_is_open(const nk_tls_stream_t* t) {
	return t && reinterpret_cast<const tls_stream*>(t)->is_open();
}

extern "C" nk_stream_t* nk_tls_stream_as_stream(nk_tls_stream_t* t) {
	if (!t) return nullptr;
	auto* base = static_cast<netkit::stream::basic_stream*>(&as_tls(t));
	return reinterpret_cast<nk_stream_t*>(base);
}

#endif /* NETKIT_WOLFSSL */
