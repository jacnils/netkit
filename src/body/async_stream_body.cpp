#include <algorithm>
#include <cstring>
#ifndef NOMINMAX
#define NOMINMAX // some windows shit
#endif
#include <netkit/body/async_stream_body.hpp>

netkit::io::task<netkit::body::read_result>
netkit::body::async_stream_body::read(char* out, std::size_t max_bytes) noexcept {
	if (max_bytes == 0)
		co_return read_result{read_status::ok, 0};

	if (remaining_ && *remaining_ == 0)
		co_return read_result{read_status::eof, 0};


	auto consume = [&](std::string& src) -> std::optional<std::size_t> {
		if (src.empty())
			return std::nullopt;

		auto n = std::min({
			max_bytes,
			src.size(),
			remaining_.value_or(src.size())
		});

		std::memcpy(
			out,
			src.data(),
			n
		);

		src.erase(0, n);

		if (remaining_)
			*remaining_ -= n;

		return n;
	};

	if (auto n = consume(buffer_))
		co_return read_result{read_status::ok, *n};

	if (auto n = consume(overflow_))
		co_return read_result{read_status::ok, *n};

	std::array<std::byte, 8192> temp{};

	auto want = std::min(
		max_bytes,
		temp.size()
	);

	if (remaining_)
		want = std::min(
			want,
			*remaining_
		);

	auto result = co_await stream_.read(
		std::span<std::byte>(
			temp.data(),
			want
		)
	);

	if (result.status == stream::stream_status::closed)
		co_return read_result{read_status::eof, 0};

	if (result.status != stream::stream_status::success)
		co_return read_result{read_status::error, 0};

	if (result.bytes == 0)
		co_return read_result{read_status::eof, 0};

	auto n = std::min(
		max_bytes,
		result.bytes
	);

	std::memcpy(
		out,
		temp.data(),
		n
	);

	if (remaining_)
		*remaining_ -= n;

	if (n < result.bytes) {
		overflow_.assign(
			reinterpret_cast<const char*>(temp.data() + n),
			result.bytes - n
		);
	}

	co_return read_result{
		read_status::ok,
		n
	};
}