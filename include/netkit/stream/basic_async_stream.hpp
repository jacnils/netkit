#pragma once

#include <netkit/stream/stream_enum.hpp>
#include <netkit/io/task.hpp>

#include <span>

namespace netkit::stream {

class NETKIT_API basic_async_stream {
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

	virtual void close() noexcept = 0;
};

}