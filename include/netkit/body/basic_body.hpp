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

		std::string read_all(std::optional<std::size_t> size = std::nullopt) {
			return netkit::body::read_all<basic_body>(*this, size);
		}
	};

	inline std::ostream& write(std::ostream& os, basic_body& body) {
		std::array<char, 8192> buffer{};

		while (true) {
			auto res = body.read(buffer.data(), buffer.size());

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

		return os;
	}

	inline std::ostream& operator<<(std::ostream& os, basic_body& body) {
		return body::write(os, body);
	}
}
