#pragma once

#include <atomic>
#include <netkit/io/io_backend.hpp>
#include <netkit/io/io_awaitable.hpp>
#include <netkit/io/task.hpp>
#include <netkit/io/cancellation.hpp>

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

		while (running_) {
			backend_.poll(-1);
			cleanup_tasks();

			if (tasks_.empty())
				break;
		}
	}

	void stop() {
		running_.store(false);
		backend_.wake();
	}

	/**
	 * @brief Wait for a file descriptor to become readable.
	 * @param fd The file descriptor to wait on
	 * @return An awaitable that suspends until the fd is readable
	 */
	io_awaitable wait_readable(io_handle_t fd) {
		return io_awaitable{backend_, fd, io_event::read};
	}

	/**
	 * @brief Wait for a file descriptor to become readable with cancellation support.
	 * @param fd The file descriptor to wait on
	 * @param token The cancellation token to check during the wait
	 * @return An awaitable that suspends until the fd is readable or is cancelled
	 * @throws cancelled_error if the token is cancelled
	 */
	io_awaitable wait_readable(io_handle_t fd, std::shared_ptr<cancellation_token> token) {
		return io_awaitable{backend_, fd, io_event::read, token};
	}

	/**
	 * @brief Wait for a file descriptor to become writable.
	 * @param fd The file descriptor to wait on
	 * @return An awaitable that suspends until the fd is writable
	 */
	io_awaitable wait_writable(io_handle_t fd) {
		return io_awaitable{backend_, fd, io_event::write};
	}

	/**
	 * @brief Wait for a file descriptor to become writable with cancellation support.
	 * @param fd The file descriptor to wait on
	 * @param token The cancellation token to check during the wait
	 * @return An awaitable that suspends until the fd is writable or is cancelled
	 * @throws cancelled_error if the token is cancelled
	 */
	io_awaitable wait_writable(io_handle_t fd, std::shared_ptr<cancellation_token> token) {
		return io_awaitable{backend_, fd, io_event::write, token};
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
