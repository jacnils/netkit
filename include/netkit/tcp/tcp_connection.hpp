#pragma once

#include <netkit/socket/native/native_sync_listener.hpp>
#include <netkit/socket/addr.hpp>
#include <netkit/stream/basic_stream.hpp>
#include <netkit/stream/socket_stream.hpp>

#include <memory>
#include <vector>

namespace netkit::tcp {

class tcp_connection {
public:
	tcp_connection(const sock::addr& addr) : stream_(std::make_unique<sock::native::native_sync_sock>(addr, sock::type::tcp)) {}

	~tcp_connection() {
		this->close();
	}

	void connect() {
		stream_.connect();
	}

	stream::stream_result read(std::span<std::byte> buffer) {
		return stream_.read(buffer);
	}

	stream::stream_result write(std::span<const std::byte> buffer) {
		return stream_.write(buffer);
	}

	stream::stream_result write_all(std::span<const std::byte> buffer) {
		std::size_t total = 0;

		while (total < buffer.size()) {
			auto result = this->write(buffer.subspan(total));

			if (result.status != stream::stream_status::success)
				return {
					result.bytes + total,
					result.status
				};

			total += result.bytes;
		}

		return {
			total,
			stream::stream_status::success
		};
	}

	stream::stream_result write_all(std::string_view data) {
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

			if (res.status == stream::stream_status::eof)
				break;

			if (res.status != stream::stream_status::success)
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

	void close() noexcept {
		stream_.close();
	}

	[[nodiscard]] sock::addr peer() const {
		return stream_.peer();
	}

	stream::socket_stream& stream() {
		return stream_;
	}

private:
	stream::socket_stream stream_;
};

}