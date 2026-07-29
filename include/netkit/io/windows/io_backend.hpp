#pragma once

// ReSharper disable once CppUnusedIncludeDirective
#include <netkit/definitions.hpp>

#if defined(NETKIT_WINDOWS) && defined(NETKIT_EPOLL)

#include <atomic>
#include <condition_variable>
#include <coroutine>
#include <mutex>
#include <netkit/io/basic_io_backend.hpp>
#include <queue>

namespace netkit::io {

class NETKIT_API io_backend : public basic_io_backend {
public:
	io_backend();
	~io_backend() override;

	void wake() override;

	void update_state(io_handle_t, const io_handle_state&) override {}

	void register_waiter(
		io_handle_t fd,
		io_event event,
		std::coroutine_handle<> h
	) override;

	void run() override;
	void stop() override;

	void poll(int timeout_ms) override;
	void poll() override;

private:
	struct waiter {
		io_handle_t fd{};
		io_event event{};
		std::coroutine_handle<> handle;
	};

	SOCKET wake_read_{INVALID_SOCKET};
	SOCKET wake_write_{INVALID_SOCKET};

	std::atomic_bool running_{true};

	std::mutex mutex_;
	std::vector<waiter> waiters_;
};

} // namespace netkit::io

#endif