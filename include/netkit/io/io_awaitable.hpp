#pragma once

#include <coroutine>
#include <cstdint>
#include <memory>
#include <netkit/io/io_backend.hpp>
#include <netkit/io/cancellation.hpp>
#include <netkit/socket/addr_type.hpp>

namespace netkit::io {
	class io_context; // fw decl.

	struct io_awaitable {
		io_backend& ctx;
		io_handle_t fd;
		io_event ev;
		std::shared_ptr<cancellation_token> token;

		io_awaitable(io_backend& ctx, io_handle_t fd, io_event ev)
			: ctx(ctx), fd(fd), ev(ev), token(nullptr) {}

		io_awaitable(io_backend& ctx, io_handle_t fd, io_event ev, std::shared_ptr<cancellation_token> token)
			: ctx(ctx), fd(fd), ev(ev), token(token) {}

		[[nodiscard]] bool await_ready() const noexcept {
			// Check if cancellation was requested before even suspending
			if (token && token->is_cancelled()) {
				return true;  // Return ready to let await_resume throw
			}
			return false;
		}

		void await_suspend(std::coroutine_handle<> h) {
			// Check again before registering with the backend
			if (token && token->is_cancelled()) {
				h.resume();  // Resume immediately to allow cancellation to be handled
				return;
			}
			ctx.register_waiter(fd, ev, h);
		}

		void await_resume() {
			if (token && token->is_cancelled()) {
				throw cancelled_error();
			}
		}
	};
}
