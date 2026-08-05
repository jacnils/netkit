#pragma once

#include <netkit/body/basic_async_body.hpp>

#include <string>
#include <cstring>

#include <netkit/stream/basic_async_stream.hpp>

namespace netkit::body {
	class NETKIT_API async_chunked_body : public basic_async_body {
	public:
		explicit async_chunked_body(stream::basic_async_stream& stream, std::string initial = {}) : stream_(stream), buffer_(std::move(initial)) {}
		io::task<read_result> read(char* out, std::size_t max_bytes) noexcept override;
	private:
		enum class state {
			read_size,
			read_data,
			consume_crlf,
			done,
		};

		io::task<bool> fill_buffer(std::size_t min_bytes) noexcept;
		io::task<std::size_t> read_raw(char* out, std::size_t bytes) noexcept;
		io::task<std::string> read_line() noexcept;

		stream::basic_async_stream& stream_;

		state state_ = state::read_size;

		std::string buffer_;
		std::size_t chunk_remaining_ = 0;
	};
}
