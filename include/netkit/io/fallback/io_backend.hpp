#pragma once

// ReSharper disable once CppUnusedIncludeDirective
#include <netkit/definitions.hpp>

#if !defined(NETKIT_LINUX) || !defined(NETKIT_EPOLL)
#if !defined(NETKIT_WINDOWS) || !defined(NETKIT_WSAPOLL)
#if !defined(NETKIT_BSD) || !defined(NETKIT_KQUEUE)
#if !defined(NETKIT_MACOS) || !defined(NETKIT_KQUEUE)

#include <atomic>
#include <condition_variable>
#include <coroutine>
#include <mutex>
#include <netkit/io/basic_io_backend.hpp>
#include <queue>
#include <thread>

namespace netkit::io {

class NETKIT_API io_backend : public basic_io_backend {
public:
	io_backend(std::size_t threads = 4);
	~io_backend() override;

	void wake() override;

	void update_state(io_handle_t, const io_handle_state&) override {}

	void register_waiter(io_handle_t fd, io_event event, std::coroutine_handle<> h) override;

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

	void worker();

	std::atomic<bool> running_{true};

	std::mutex mutex_;
	std::condition_variable cv_;

	std::queue<waiter> queue_;
	std::vector<std::thread> workers_;
	std::vector<waiter> waiters_;
};

} // namespace netkit::io

#endif
#endif
#endif
#endif