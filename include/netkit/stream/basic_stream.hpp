#pragma once

#include <netkit/stream/stream_enum.hpp>

namespace netkit::stream {

class basic_stream {
public:
	virtual ~basic_stream() = default;

	virtual stream_result read(std::span<std::byte> buffer) = 0;
	virtual stream_result write(std::span<const std::byte> buffer) = 0;

	virtual void close() noexcept = 0;
};

}