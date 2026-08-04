#pragma once

#include <algorithm>
#include <netkit/body/basic_async_body.hpp>
#include <netkit/socket/native/basic_native_async_sock.hpp>
#include <optional>
#include <string>

#include <netkit/stream/basic_async_stream.hpp>

namespace netkit::body {

class NETKIT_API async_stream_body : public basic_async_body {
public:
	async_stream_body(stream::basic_async_stream& stream, std::optional<std::size_t> length, std::string initial = {})
		: stream_(stream),
		  remaining_(length),
		  buffer_(std::move(initial))
	{}

	io::task<read_result> read(char* out, std::size_t max_bytes) noexcept override;

private:
	stream::basic_async_stream& stream_;
	std::optional<std::size_t> remaining_;
	std::string buffer_;
	std::string overflow_;
};

}
