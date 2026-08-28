#pragma once

#include <netkit/stream/basic_stream.hpp>

#include <algorithm>
#include <cstring>
#include <netkit/body/basic_body.hpp>
#include <netkit/socket/native/basic_native_sync_socket.hpp>
#include <optional>
#include <string>

namespace netkit::body {

class NETKIT_API stream_body : public basic_body {
public:
	stream_body(stream::basic_stream& stream, std::optional<std::size_t> length, std::string initial = {})
		: stream_(stream),
		  remaining_(length),
		  buffer_(std::move(initial))
	{}

	read_result read(char* out, std::size_t max_bytes) noexcept override;

private:
	stream::basic_stream& stream_;
	std::optional<std::size_t> remaining_;
	std::string buffer_;
	std::string overflow_;
};

}