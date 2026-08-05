#pragma once

#include <netkit/export.hpp>
#include <netkit/io/task.hpp>

#include <string>
#include <cstddef>
#include <memory>

namespace netkit::body {
	class basic_body;
	class basic_async_body;

	enum class read_status {
		ok,
		eof,
		error,
		timeout
	};

	class NETKIT_API read_result {
	public:
		virtual ~read_result() = default;
		read_result() = default;
		read_result(read_status status, std::size_t bytes_read) : _status(status), _bytes_read(bytes_read) {}

		[[nodiscard]] read_status get_status() const { return _status; }
		[[nodiscard]] std::size_t get_bytes_read() const { return _bytes_read; }
	private:
		read_status _status{};
		std::size_t _bytes_read{};
	};

	template <typename T, typename... Args>
	requires std::derived_from<T, basic_body> && std::constructible_from<T, Args...>
	std::unique_ptr<basic_body> make_body(Args&&... args) {
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template <typename T, typename... Args>
	requires std::derived_from<T, basic_async_body> && std::constructible_from<T, Args...>
	std::unique_ptr<basic_async_body> make_body(Args&&... args) {
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template <typename T>
	concept Readable = requires(T& obj, char* buffer, std::size_t size) {
		{ obj.read(buffer, size) } -> std::same_as<read_result>;
	};

	template <typename T>
	concept AsyncReadable = requires(T& obj, char* buffer, std::size_t size) {
		{ obj.read(buffer, size) } -> std::same_as<io::task<read_result>>;
	};

	template<Readable T> std::string read_all(T& reader, std::optional<std::size_t> size = {}) {
		std::string result;

		if (size)
			result.reserve(*size);

		while (true) {
			char buffer[8192];
			auto res = reader.read(buffer, sizeof(buffer));

			if (res.get_status() == netkit::body::read_status::eof)
				break;

			if (res.get_status() == netkit::body::read_status::error)
				throw std::runtime_error("read failed");

			if (res.get_status() == netkit::body::read_status::timeout)
				continue;

			auto bytes = res.get_bytes_read();

			if (bytes > sizeof(buffer))
				throw std::runtime_error("invalid read size");

			result.append(buffer, bytes);
		}

		return result;
	}

	template <AsyncReadable T> io::task<std::string> read_all(T& reader, std::optional<std::size_t> size = {}) {
		std::string result;

		if (size)
			result.reserve(*size);

		while (true) {
			char buffer[8192];
			auto res = co_await reader.read(buffer, sizeof(buffer));

			if (res.get_status() == netkit::body::read_status::eof)
				break;

			if (res.get_status() == netkit::body::read_status::error)
				throw std::runtime_error("read failed");

			if (res.get_status() == netkit::body::read_status::timeout)
				continue;

			auto bytes = res.get_bytes_read();

			if (bytes > sizeof(buffer))
				throw std::runtime_error("invalid read size");

			result.append(buffer, bytes);
		}

		co_return result;
	}
}
