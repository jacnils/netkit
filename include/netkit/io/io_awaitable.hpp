#pragma once

#include <coroutine>
#include <cstdint>
#include <netkit/io/linux/io_backend.hpp>
#include <netkit/socket/addr_type.hpp>

namespace netkit::io {
	class io_context; // fw decl.

	struct io_awaitable {
		io_backend& ctx;
		sock::fd_t fd;
		io_event ev;

		[[nodiscard]] bool await_ready() const noexcept {
			return false;
		}

		void await_suspend(std::coroutine_handle<> h) {
			ctx.register_waiter(fd, ev, h);
		}

		void await_resume() noexcept {}
	};
}