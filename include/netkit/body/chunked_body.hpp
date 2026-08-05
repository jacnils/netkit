#pragma once

#include <netkit/body/basic_body.hpp>

#include <string>
#include <cstring>

#include <netkit/stream/basic_stream.hpp>

namespace netkit::body {
	class NETKIT_API chunked_body : public basic_body {
	public:
		explicit chunked_body(stream::basic_stream& stream, std::string initial = {}) : stream_(stream), buffer_(std::move(initial)) {}
		read_result read(char* out, std::size_t max_bytes) noexcept override;
	private:
		enum class state {
			read_size,
			read_data,
			consume_crlf,
			done,
		};

		bool fill_buffer(std::size_t min_bytes) noexcept;
		std::size_t read_raw(char* out, std::size_t bytes) noexcept;
		std::string read_line() noexcept;

		stream::basic_stream& stream_;

		state state_ = state::read_size;

		std::string buffer_;
		std::size_t chunk_remaining_ = 0;
	};
}
