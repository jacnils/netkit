#pragma once

#include <netkit/socket/addr.hpp>
#include <netkit/stream/stream_enum.hpp>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

namespace netkit::stream {

class basic_stream {
public:
	virtual ~basic_stream() = default;

	virtual stream_result read(std::span<std::byte> buffer) = 0;
	virtual stream_result write(std::span<const std::byte> buffer) = 0;

	stream_result read(void* data, std::size_t size) {
		return this->read(std::span(static_cast<std::byte*>(data), size));
	}

	stream_result write(const void* data, std::size_t size) {
		return this->write(std::span(static_cast<const std::byte*>(data), size));
	}

	stream_result write_all(std::span<const std::byte> buffer) {
		std::size_t total = 0;

		while (total < buffer.size()) {
			auto result = this->write(buffer.subspan(total));

			if (result.status != stream_status::success)
				return {
					result.bytes + total,
					result.status
				};

			total += result.bytes;
		}

		return {
			total, stream_status::success
		};
	}

	stream_result write_all(std::string_view data) {
		return write_all(
			std::as_bytes(
				std::span(data.data(), data.size())
			)
		);
	}

	std::vector<std::byte> read_all(std::size_t max_bytes = 16 * 1024 * 1024) {
		std::vector<std::byte> result;

		std::size_t total = 0;

		std::array<std::byte, 8192> buffer{};

		while (total < max_bytes) {
			auto res = read(buffer);

			if (res.status == stream_status::eof)
				break;

			if (res.status != stream_status::success)
				throw std::runtime_error("read failed");

			auto amount = std::min(res.bytes, max_bytes - total);

			result.insert(
				result.end(),
				buffer.begin(),
				buffer.begin() + amount
			);

			total += amount;

			if (amount != res.bytes)
				throw std::length_error("read_all exceeded maximum size");
		}

		return result;
	}

	std::string read_all_string() {
		auto data = this->read_all();

		return {
			reinterpret_cast<const char*>(data.data()),
			data.size()
		};
	}

	virtual void close() noexcept = 0;

	virtual std::optional<sock::addr> get_addr() {
		return {};
	}
};

}