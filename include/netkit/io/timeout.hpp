#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <netkit/io/cancellation.hpp>
#include <netkit/io/task.hpp>
#include <thread>

namespace netkit::io {

/**
 * @brief Exception thrown when an operation times out.
 */
class timeout_error : public std::runtime_error {
public:
	timeout_error() : std::runtime_error("Operation timed out") {}
};

/**
 * @brief Set the cancellation token for the current coroutine context.
 * @internal Used by timeout() to make the token available to nested operations.
 */
void set_current_cancellation_token(std::shared_ptr<cancellation_token> token) noexcept;

/**
 * @brief Get the cancellation token for the current coroutine context.
 * @return The cancellation token if set, nullptr otherwise.
 */
[[nodiscard]] std::shared_ptr<cancellation_token> get_current_cancellation_token() noexcept;

/**
 * @brief Check if cancellation has been requested for the current context.
 * @throws cancelled_error if cancellation was requested.
 * 
 * This is a cooperative cancellation point. Tasks should call this periodically
 * to allow cancellation during long-running operations.
 * 
 * @example
 * netkit::io::task<void> long_operation() {
 *     for (int i = 0; i < 1000; ++i) {
 *         netkit::io::check_cancellation();  // Allow cancellation here
 *         do_work();
 *     }
 * }
 */
void check_cancellation();

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
 * @param token Optional cancellation token. If not provided, uses thread-local token.
 * @return A task that completes after the duration (or on cancellation)
 * 
 * @throws cancelled_error if cancelled during the sleep
 * 
 * @example
 * auto result = co_await netkit::io::timeout(
 *     []() -> netkit::io::task<int> {
 *         co_await netkit::io::async_sleep(std::chrono::seconds(5));
 *         co_return 42;
 *     }(),
 *     std::chrono::seconds(2)
 * );
 */
template<typename Rep, typename Period>
task<> async_sleep(std::chrono::duration<Rep, Period> duration, std::shared_ptr<cancellation_token> token = nullptr) {
	if (!token) {
		token = get_current_cancellation_token();
	}
	co_await sleep_awaitable<Rep, Period>{duration, token};
}

/**
 * @brief Wraps a task with a timeout.
 * 
 * If the task does not complete within the specified duration, it will be:
 * 1. Marked as cancelled
 * 2. Forcefully destroyed if it doesn't respond to cancellation
 * 3. A timeout_error will be thrown
 * 
 * @tparam T The return type of the task
 * @tparam Rep The representation type of the duration
 * @tparam Period The period type of the duration
 * @param t The task to wrap
 * @param duration The timeout duration
 * @return A task that will complete or timeout
 * 
 * @throws timeout_error if the operation exceeds the specified duration
 * 
 * @example
 * // For cooperative tasks (call check_cancellation() or use I/O):
 * auto result = co_await io::timeout(
 *     async_operation(),
 *     std::chrono::seconds(5)
 * );
 * 
 * // For non-cooperative tasks, forceful termination will occur after timeout:
 * auto result = co_await io::timeout(
 *     blocking_operation(),
 *     std::chrono::seconds(5)
 * );
 */
template<typename T, typename Rep, typename Period>
task<T> timeout(task<T> t, std::chrono::duration<Rep, Period> duration, std::function<void()> handler = std::function<void()>()) {
	cancellation_source source;
	t.set_cancellation_token(source.get_token());

	auto handle = t.get_handle_if_available();

	std::thread timer_thread([&source, handle, duration, handler]() {
		std::this_thread::sleep_for(duration);
		source.cancel();

		if (handle && !handle.done()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			if (!handle.done()) {
				handle.destroy();

				if (handler)
					handler();
			}
		}
	});

	timer_thread.detach();

	set_current_cancellation_token(source.get_token());

	try {
		co_return co_await std::move(t);
	} catch (const cancelled_error&) {
		handler();
	}
}

} // namespace netkit::io
