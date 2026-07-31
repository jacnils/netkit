#pragma once

#include <atomic>
#include <netkit/io/io_backend.hpp>
#include <netkit/io/io_awaitable.hpp>
#include <netkit/io/task.hpp>

namespace netkit::io {

class NETKIT_API io_context {
public:
	void run() {
		running_ = true;

		while (running_) {
			backend_.poll(!tasks_.empty() ? 0 : -1);

			cleanup_tasks();
		}
	}

	void run_until_idle() {
		running_ = true;

		while (running_ && !tasks_.empty()) {
			backend_.poll(!tasks_.empty() ? 0 : -1);

			cleanup_tasks();
		}
	}

	void stop() {
		running_.store(false);
		backend_.wake();
	}

	io_awaitable wait_readable(io_handle_t fd) {
		return io_awaitable{backend_, fd, io_event::read};
	}

	io_awaitable wait_writable(io_handle_t fd) {
		return io_awaitable{backend_, fd, io_event::write};
	}

	void spawn(task<void>&& t) {
		tasks_.push_back(std::move(t));
		tasks_.back().resume();
	}
private:
	void cleanup_tasks() {
		std::erase_if(tasks_, [](const auto& task) { return task.done(); } );
	}

	io_backend backend_;
	std::atomic_bool running_ = false;

	std::vector<task<void>> tasks_;
};

}