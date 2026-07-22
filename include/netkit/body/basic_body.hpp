#pragma once

#include <cstddef>
#include <optional>

#include <netkit/export.hpp>

namespace netkit::body {
	enum class NETKIT_API read_status {
		ok,
		eof,
		error,
		timeout
	};

	class NETKIT_API read_result {
	public:
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
		[[nodiscard]] virtual std::optional<std::size_t> size() const {
			return std::nullopt;
		}
		virtual bool rewind() {
			return false;
		}
	};
}