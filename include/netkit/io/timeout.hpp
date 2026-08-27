#pragma once

#include <chrono>
#include <thread>
#include <netkit/io/task.hpp>
#include <netkit/io/cancellation.hpp>

namespace netkit::io {

/**
 * @brief Exception thrown when an operation times out.
 */
class timeout_error : public std::runtime_error {
public:
	timeout_error() : std::runtime_error("Operation timed out") {}
};

/**
 * @brief An awaitable that yields control for a specified duration.
 * 
 * This allows the coroutine to be cancelled during the sleep, unlike
 * std::this_thread::sleep_for which blocks and ignores cancellation.
 */
template<typename Rep, typename Period>
struct sleep_awaitable {
	std::chrono::duration<Rep, Period> duration;
	std::shared_ptr<cancellation_token> token;
	std::thread timer_thread;

	[[nodiscard]] bool await_ready() const noexcept {
		if (token && token->is_cancelled()) {
			return true;
		}
		return false;
	}

	void await_suspend(std::coroutine_handle<> h) {
		if (token && token->is_cancelled()) {
			h.resume();
			return;
		}

		// Spawn a timer thread that will resume the coroutine after the duration
		timer_thread = std::thread([this, h]() {
			std::this_thread::sleep_for(duration);
			if (!(token && token->is_cancelled())) {
				h.resume();
			}
		});
	}

	void await_resume() {
		if (timer_thread.joinable()) {
			timer_thread.join();
		}

		if (token && token->is_cancelled()) {
			throw cancelled_error();
		}
	}
};

/**
 * @brief Async sleep that can be cancelled via a cancellation token.
 * 
 * @tparam Rep The representation type of the duration
 * @tparam Period The period type of the duration
 * @param duration The duration to sleep
 * @param token Optional cancellation token. If not provided, will not be cancellable.
 * @return A task that completes after the duration (or on cancellation)
 * 
 * @throws cancelled_error if cancelled during the sleep
 * 
 * @example
 * auto source = netkit::io::cancellation_source();
 * try {
 *     co_await netkit::io::async_sleep(std::chrono::seconds(5), source.get_token());
 * } catch (const netkit::io::cancelled_error&) {
 *     // Sleep was cancelled
 * }
 */
template<typename Rep, typename Period>
task<void> async_sleep(
	std::chrono::duration<Rep, Period> duration,
	std::shared_ptr<cancellation_token> token = nullptr
) {
	co_await sleep_awaitable<Rep, Period>{duration, token};
}

/**
 * @brief Wraps a task with a timeout.
 * 
 * If the task does not complete within the specified duration, it will be cancelled
 * and a timeout_error will be thrown.
 * 
 * @tparam T The return type of the task
 * @tparam Rep The representation type of the duration
 * @tparam Period The period type of the duration
 * @param t The task to wrap
 * @param duration The timeout duration
 * @return A task that will complete or timeout
 * 
 * @throws timeout_error if the operation exceeds the specified duration
 * @throws cancelled_error if the operation is cancelled
 * 
 * @example
 * auto result = co_await io::timeout(
 *     socket.read_async(),
 *     std::chrono::seconds(5)
 * );
 */
template<typename T, typename Rep, typename Period>
task<T> timeout(task<T> t, std::chrono::duration<Rep, Period> duration) {
	// Create a cancellation source for this timeout
	cancellation_source source;
	t.set_cancellation_token(source.get_token());

	// Spawn a timer thread that will cancel after the specified duration
	std::thread timer_thread([source, duration]() {
		std::this_thread::sleep_for(duration);
		source.cancel();
	});
	timer_thread.detach();

	try {
		// Await the task - if it completes before timeout, great
		// If timeout fires, the task will throw cancelled_error
		co_return co_await std::move(t);
	} catch (const cancelled_error&) {
		// Re-throw as timeout_error for clarity
		throw timeout_error();
	}
}

} // namespace netkit::io
