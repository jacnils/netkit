#include <iostream>
#include <netkit/export.hpp>
#include <netkit/io/fallback/io_backend.hpp>

#if !defined(NETKIT_LINUX) || !defined(NETKIT_EPOLL)
#if !defined(NETKIT_WINDOWS) || !defined(NETKIT_WSAPOLL)

void netkit::io::io_backend::worker() {
	while (running_) {
		waiter task;

		{
			std::unique_lock lock(mutex_);

			cv_.wait(lock, [&] {
				return !queue_.empty() || !running_;
			});

			if (!running_)
				return;

			task = queue_.front();
			queue_.pop();
		}

		if (task.handle)
			task.handle.resume();
	}
}

netkit::io::io_backend::io_backend(std::size_t count) {
	for (std::size_t i = 0; i < count; ++i) {
		workers_.emplace_back(
			[this] {
				worker();
			}
		);
	}
}

netkit::io::io_backend::~io_backend() {
	this->io_backend::stop();

	for (auto& thread : workers_) {
		if (thread.joinable())
			thread.join();
	}
}

void netkit::io::io_backend::wake() {
	cv_.notify_all();
}

void netkit::io::io_backend::register_waiter(io_handle_t fd, io_event event, std::coroutine_handle<> handle) {
	std::lock_guard lock(mutex_);

	waiters_.push_back({
		fd,
		event,
		handle
	});

	cv_.notify_one();
}


void netkit::io::io_backend::run() {
	while (running_) {
		poll();
	}
}

void netkit::io::io_backend::stop() {
	running_ = false;
	cv_.notify_all();
}

void netkit::io::io_backend::poll(int timeout_ms) {
	std::vector<waiter> pending;

	{
		std::unique_lock lock(mutex_);

		if (waiters_.empty()) {
			cv_.wait_for(
				lock,
				std::chrono::milliseconds(timeout_ms)
			);
		}

		pending.swap(waiters_);
	}

	for (auto& waiter : pending) {
		if (waiter.handle)
			waiter.handle.resume();
	}
}


void netkit::io::io_backend::poll() {
	poll(-1);
}

#endif
#endif