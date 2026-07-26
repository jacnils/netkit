#pragma once

#include <netkit/stream/stream_enum.hpp>

#include <span>

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

	virtual void close() noexcept = 0;
};

}