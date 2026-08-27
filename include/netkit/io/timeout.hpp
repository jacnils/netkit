#pragma once

#include <chrono>
#include <thread>
#include <netkit/io/task.hpp>
#include <netkit/io/cancellation.hpp>

namespace netkit::io {

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

/**
 * @brief Exception thrown when an operation times out.
 */
class timeout_error : public std::runtime_error {
public:
	timeout_error() : std::runtime_error("Operation timed out") {}
};

} // namespace netkit::io
