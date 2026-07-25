#pragma once

#include <netkit/io/task.hpp>
#include <netkit/definitions.hpp>
#include <netkit/io/basic_io_context.hpp>
#include <netkit/io/io_awaitable.hpp>

#include <atomic>

#ifdef NETKIT_LINUX

namespace netkit::io {

class io_context {
public:
	void run() {
		running_ = true;

		while (running_) {
			backend_.poll(-1);

			cleanup_tasks();
		}
	}

	void run_until_idle() {
		running_ = true;

		while (running_ && !tasks_.empty()) {
			backend_.poll(-1);

			cleanup_tasks();
		}
	}

	void stop() {
		running_.store(false);
		backend_.wake();
	}

	io_awaitable wait_readable(int fd) {
		return io_awaitable{backend_, fd, io_event::read};
	}

	io_awaitable wait_writable(int fd) {
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

	basic_io_context backend_;
	std::atomic_bool running_ = false;

	std::vector<task<void>> tasks_;
};

}

#endif