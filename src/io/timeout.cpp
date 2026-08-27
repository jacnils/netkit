#include <netkit/io/timeout.hpp>

namespace netkit::io {

namespace detail {
	inline thread_local std::shared_ptr<cancellation_token> tls_cancellation_token;
}

void set_current_cancellation_token(std::shared_ptr<cancellation_token> token) noexcept {
	detail::tls_cancellation_token = token;
}

[[nodiscard]] std::shared_ptr<cancellation_token> get_current_cancellation_token() noexcept {
	return detail::tls_cancellation_token;
}

void check_cancellation() {
	auto token = get_current_cancellation_token();
	if (token && token->is_cancelled()) {
		throw cancelled_error();
	}
}

} // namespace netkit::io
