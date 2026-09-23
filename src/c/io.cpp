#include "internal.hpp"

#include <netkit/c/io.h>

#include <netkit/io/cancellation.hpp>
#include <netkit/io/io_context.hpp>

using netkit::io::cancellation_source;
using netkit::io::cancellation_token;
using netkit::io::io_context;

namespace {
	io_context& as_ctx(nk_io_context_t* c) { return *reinterpret_cast<io_context*>(c); }
	nk_io_context_t* to_c(io_context* c) { return reinterpret_cast<nk_io_context_t*>(c); }

	cancellation_source& as_source(nk_cancellation_source_t* s) {
		return *reinterpret_cast<cancellation_source*>(s);
	}
	const cancellation_source& as_source(const nk_cancellation_source_t* s) {
		return *reinterpret_cast<const cancellation_source*>(s);
	}
	nk_cancellation_source_t* to_c(cancellation_source* s) { return reinterpret_cast<nk_cancellation_source_t*>(s); }

	cancellation_token& as_token(nk_cancellation_token_t* t) { return *reinterpret_cast<cancellation_token*>(t); }
	const cancellation_token& as_token(const nk_cancellation_token_t* t) {
		return *reinterpret_cast<const cancellation_token*>(t);
	}
	nk_cancellation_token_t* to_c(cancellation_token* t) { return reinterpret_cast<nk_cancellation_token_t*>(t); }
}

extern "C" nk_status_t nk_io_context_create(nk_io_context_t** out) {
	return netkit::c::wrap_checked(out != nullptr, [&] { *out = to_c(new io_context()); });
}

extern "C" void nk_io_context_destroy(nk_io_context_t* ctx) {
	if (!ctx) return;
	as_ctx(ctx).stop();
	delete reinterpret_cast<io_context*>(ctx);
}

extern "C" nk_status_t nk_io_context_run(nk_io_context_t* ctx) {
	return netkit::c::wrap_checked(ctx != nullptr, [&] { as_ctx(ctx).run(); });
}

extern "C" nk_status_t nk_io_context_run_until_idle(nk_io_context_t* ctx) {
	return netkit::c::wrap_checked(ctx != nullptr, [&] { as_ctx(ctx).run_until_idle(); });
}

extern "C" void nk_io_context_stop(nk_io_context_t* ctx) {
	if (ctx) as_ctx(ctx).stop();
}

extern "C" nk_status_t nk_cancellation_source_create(nk_cancellation_source_t** out) {
	return netkit::c::wrap_checked(out != nullptr, [&] { *out = to_c(new cancellation_source()); });
}

extern "C" void nk_cancellation_source_destroy(nk_cancellation_source_t* s) {
	delete reinterpret_cast<cancellation_source*>(s);
}

extern "C" void nk_cancellation_source_cancel(nk_cancellation_source_t* s) {
	if (s) as_source(s).cancel();
}

extern "C" bool nk_cancellation_source_is_cancelled(const nk_cancellation_source_t* s) {
	return s && as_source(s).is_cancelled();
}

extern "C" nk_status_t nk_cancellation_token_create(nk_cancellation_token_t** out) {
	return netkit::c::wrap_checked(out != nullptr, [&] { *out = to_c(new cancellation_token()); });
}

extern "C" void nk_cancellation_token_destroy(nk_cancellation_token_t* t) {
	delete reinterpret_cast<cancellation_token*>(t);
}

extern "C" void nk_cancellation_token_cancel(nk_cancellation_token_t* t) {
	if (t) as_token(t).cancel();
}

extern "C" bool nk_cancellation_token_is_cancelled(const nk_cancellation_token_t* t) {
	return t && as_token(t).is_cancelled();
}

extern "C" void nk_cancellation_token_reset(nk_cancellation_token_t* t) {
	if (t) as_token(t).reset();
}
