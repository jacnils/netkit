#pragma once

#include <cstddef>
#include <optional>
#include <ostream>
#include <string>
#include <array>
#include <span>

#include <netkit/export.hpp>
#include <netkit/body/read_status_enum.hpp>

namespace netkit::body {
	class NETKIT_API basic_async_body {
	public:
		virtual ~basic_async_body() = default;

		virtual netkit::io::task<read_result> read(char* buffer, std::size_t max_bytes) noexcept = 0;

		io::task<read_result> read(std::span<char> buffer) noexcept {
			co_return co_await read(buffer.data(), buffer.size());
		}

		[[nodiscard]] virtual std::optional<std::size_t> size() const {
			return std::nullopt;
		}

		[[nodiscard]] bool empty() const {
			auto sz = size();
			if (sz.has_value()) {
				return sz.value() == 0;
			}
			return false;
		}

		virtual bool rewind() {
			return false;
		}

		io::task<std::string> read_all(std::optional<std::size_t> size = std::nullopt) {
			co_return co_await body::read_all<basic_async_body>(*this, size);
		}
	};

	inline io::task<void> write(std::ostream& os, basic_async_body& body) {
		std::array<char, 8192> buffer{};

		while (true) {
			auto res = co_await body.read(buffer.data(), buffer.size());

			if (res.get_status() == read_status::error) {
				os.setstate(std::ios::badbit);
				break;
			}

			if (res.get_bytes_read() > 0) {
				os.write(buffer.data(), static_cast<std::streamsize>(res.get_bytes_read()));
			}

			if (res.get_status() == read_status::eof) {
				break;
			}
		}
	}

	inline netkit::io::task<void> write_to(std::ostream& os, basic_async_body& body) {
		co_await body::write(os, body);
	}
}
