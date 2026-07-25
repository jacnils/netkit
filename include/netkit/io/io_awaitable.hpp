#pragma once

#include <coroutine>
#include <cstdint>
#include <netkit/io/basic_io_context.hpp>
#include <netkit/socket/addr_type.hpp>

#ifdef NETKIT_LINUX

namespace netkit::io {
	class io_context; // fw decl.

	struct io_awaitable {
		basic_io_context& ctx;
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

#endif