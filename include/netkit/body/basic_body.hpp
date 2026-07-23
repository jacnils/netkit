#pragma once

#include <cstddef>
#include <optional>
#include <stdexcept>

#include <netkit/export.hpp>
#include <optional>
#include <stdexcept>

namespace netkit::body {
	enum class NETKIT_API read_status {
		ok,
		eof,
		error,
		timeout
	};

	template<typename T>
	concept Readable = requires(T& obj, char* buffer, std::size_t size) {
		{ obj.read(buffer, size) };
	};

	template<Readable T> std::string read_all(T& reader, std::optional<std::size_t> size = {}) {
		std::string result;

		if (size)
			result.reserve(*size);

		char buffer[8192];

		while (true) {
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

	class NETKIT_API read_result {
	public:
		virtual ~read_result() = default;
		read_result() = default;
		read_result(read_status status, std::size_t bytes_read)
			: _status(status), _bytes_read(bytes_read) {}

		[[nodiscard]] read_status get_status() const { return _status; }
		[[nodiscard]] std::size_t get_bytes_read() const { return _bytes_read; }
	private:
		read_status _status{};
		std::size_t _bytes_read{};
	};

	class NETKIT_API basic_body {
	public:
		virtual ~basic_body() = default;

		virtual read_result read(char* buffer, std::size_t max_bytes) noexcept = 0;

		read_result read(std::span<char> buffer) noexcept {
			auto res = read(buffer.data(), buffer.size());
			return res;
		}
		
		[[nodiscard]] virtual std::optional<std::size_t> size() const {
			return std::nullopt;
		}
		virtual bool rewind() {
			return false;
		}

		std::string read_all(std::optional<std::size_t> size = std::nullopt) {
			return netkit::body::read_all<basic_body>(*this, size);
		}
	};
}