#pragma once

#include <atomic>
#include <memory>
#include <coroutine>

namespace netkit::io {

/**
 * @brief Exception thrown when a task is cancelled.
 */
class cancelled_error : public std::runtime_error {
public:
	cancelled_error() : std::runtime_error("Task was cancelled") {}
};

/**
 * @brief A shared cancellation token that can be used to cancel an operation.
 * @note Thread-safe for checking cancellation status.
 */
class cancellation_token {
public:
	cancellation_token() = default;

	/**
	 * @brief Check if cancellation has been requested.
	 * @return true if cancellation was requested, false otherwise.
	 */
	[[nodiscard]] bool is_cancelled() const noexcept {
		return cancelled_.load(std::memory_order_acquire);
	}

	/**
	 * @brief Request cancellation.
	 * @note Does not immediately stop the operation, but signals it should stop.
	 */
	void cancel() noexcept {
		cancelled_.store(true, std::memory_order_release);
	}

	/**
	 * @brief Reset the cancellation flag.
	 */
	void reset() noexcept {
		cancelled_.store(false, std::memory_order_release);
	}

private:
	std::atomic<bool> cancelled_{false};
};

/**
 * @brief A cancellation source that owns a cancellation token.
 * @note Can be used to cancel operations from outside the coroutine.
 */
class cancellation_source {
public:
	cancellation_source() : token_(std::make_shared<cancellation_token>()) {}

	/**
	 * @brief Get the associated cancellation token.
	 * @return A shared reference to the cancellation token.
	 */
	std::shared_ptr<cancellation_token> get_token() const noexcept {
		return token_;
	}

	/**
	 * @brief Request cancellation.
	 */
	void cancel() noexcept {
		token_->cancel();
	}

	/**
	 * @brief Check if cancellation has been requested.
	 * @return true if cancellation was requested, false otherwise.
	 */
	[[nodiscard]] bool is_cancelled() const noexcept {
		return token_->is_cancelled();
	}

private:
	std::shared_ptr<cancellation_token> token_;
};

} // namespace netkit::io
