#pragma once

#include <coroutine>
#include <exception>
#include <iostream>
#include <utility>
#include <type_traits>

namespace netkit::io {

template<typename T = void>
class task;

template<typename T>
struct promise_return {
	std::optional<T> value;

	template<typename U>
	void return_value(U&& v) {
		value.emplace(std::forward<U>(v));
	}
};

template<>
struct promise_return<void> {
	void return_void() {}
};

template<typename T>
class task {
public:
	struct promise_type : promise_return<T> {
		std::exception_ptr exception;
		std::coroutine_handle<> continuation{};

		task get_return_object() {
			return task{
				handle_type::from_promise(*this)
			};
		}

		std::suspend_always initial_suspend() noexcept {
			return {};
		}

		auto final_suspend() noexcept {
			struct final_awaiter {
				bool await_ready() noexcept {
					return false;
				}

				std::coroutine_handle<> await_suspend(handle_type h) noexcept {
					if (auto _continuation = h.promise().continuation)
						return _continuation;

					return std::noop_coroutine();
				}

				void await_resume() noexcept {}
			};

			return final_awaiter{};
		}

		void unhandled_exception() {
			exception = std::current_exception();
		}
	};

	using handle_type = std::coroutine_handle<promise_type>;

	class awaiter {
	public:
		explicit awaiter(handle_type h) : handle_(std::exchange(h, {})) {}

		[[nodiscard]] bool await_ready() const noexcept {
			return handle_.done();
		}

		void await_suspend(std::coroutine_handle<> caller) {
			handle_.promise().continuation = caller;
			handle_.resume();
		}

		auto await_resume() {
			if (handle_.promise().exception)
				std::rethrow_exception(
					handle_.promise().exception
				);

			if constexpr (std::is_void_v<T>) {
				handle_.destroy();
				handle_ = {};
			}
			else {
				auto value = std::move(
					*handle_.promise().value
				);

				handle_.destroy();
				handle_ = {};

				return value;
			}
		}
	private:
		handle_type handle_;
	};

	auto operator co_await() && {
		return awaiter{
			std::exchange(handle_, {})
		};
	}

	void resume() {
		if (handle_ && !handle_.done()) {
			handle_.resume();
		}
	}

	[[nodiscard]] bool done() const noexcept {
		return !handle_ || handle_.done();
	}

	task(task&& other) noexcept : handle_(std::exchange(other.handle_, {})) {}

	task& operator=(task&& other) noexcept {
		if (this != &other) {
			destroy();
			handle_ = std::exchange(other.handle_, {});
		}

		return *this;
	}

	~task() {
		destroy();
	}
private:
	explicit task(handle_type h) : handle_(h) {}

	void destroy() {
		if (handle_)
			handle_.destroy();
	}

	handle_type handle_;
};

}