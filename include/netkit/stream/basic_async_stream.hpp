#pragma once

#include <netkit/stream/stream_enum.hpp>
#include <netkit/io/task.hpp>
#include <netkit/socket/addr.hpp>

#include <netkit/body/basic_async_body.hpp>

#include <span>
#include <stdexcept>
#include <vector>
#include <string>
#include <string_view>
#include <array>
#include <cstddef>
#include <optional>

namespace netkit::stream {

class basic_async_stream {
public:
	virtual ~basic_async_stream() = default;

	virtual io::task<stream_result> read(std::span<std::byte> buffer) = 0;
	virtual io::task<stream_result> write(std::span<const std::byte> buffer) = 0;

	virtual io::task<stream_result> read(void* data, std::size_t size) {
		co_return co_await read(std::span(static_cast<std::byte*>(data), size) );
	}

	virtual io::task<stream_result> write(const void* data, std::size_t size) {
		co_return co_await write(std::span(static_cast<const std::byte*>(data), size));
	}

	io::task<stream_result>
	write_all(std::span<const std::byte> buffer) {
		std::size_t total = 0;

		while (total < buffer.size()) {
			auto result = co_await write(
				buffer.subspan(total)
			);

			if (result.status != stream::stream_status::success)
				co_return result;

			total += result.bytes;
		}

		co_return stream::stream_result{
			total,
			stream::stream_status::success
		};
	}

	io::task<stream_result> write_all(std::string_view data) {
		co_return co_await this->write_all(std::as_bytes(std::span(data.data(), data.size())));
	}

	io::task<stream_result> write_all(const void* data, std::size_t size) {
		co_return co_await write_all(std::span(static_cast<const std::byte*>(data), size));
	}

	io::task<stream_result> write_all(body::basic_async_body& body) {
		char buf[4096];
		size_t bytes_read = 0;

		while (true) {
			auto res = co_await body.read(buf, sizeof(buf));

			if (res.get_status() == body::read_status::error)
				co_return stream_result{bytes_read, stream_status::error};

			if (res.get_bytes_read() > 0) {
				auto write_res = co_await this->write_all(buf, res.get_bytes_read());

				if (write_res.status != stream_status::success)
					co_return stream_result{bytes_read, write_res.status};

				bytes_read += res.get_bytes_read();
			}

			if (res.get_status() == body::read_status::eof)
				break;
		}

		co_return stream_result{
			bytes_read,
			stream_status::success
		};
	}

	io::task<std::vector<std::byte>> read_all(std::size_t max_bytes = 16 * 1024 * 1024) {
		std::vector<std::byte> result;

		std::array<std::byte, 8192> buffer{};

		std::size_t total = 0;

		while (total < max_bytes) {
			auto res = co_await read(buffer);

			if (res.status == stream::stream_status::eof)
				break;

			if (res.status != stream::stream_status::success)
				throw std::runtime_error("read failed");

			result.insert(
				result.end(),
				buffer.begin(),
				buffer.begin() + res.bytes
			);

			total += res.bytes;
		}

		co_return result;
	}

	io::task<std::string> read_all_string() {
		auto data = co_await this->read_all();

		co_return std::string{
			reinterpret_cast<const char*>(data.data()),
			data.size()
		};
	}

	virtual void close() noexcept = 0;

	[[nodiscard]] virtual bool is_open() const noexcept = 0;

	virtual std::optional<socket::addr> get_addr() {
		return {};
	}
};

}