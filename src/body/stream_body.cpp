#include <algorithm>
#define NOMINMAX // some windows shit
#include <netkit/body/stream_body.hpp>

netkit::body::read_result
netkit::body::stream_body::read(char* out, std::size_t max_bytes) noexcept {
	if (max_bytes == 0)
		return {read_status::ok, 0};

	if (remaining_ && *remaining_ == 0)
		return {read_status::eof, 0};


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
		return {read_status::ok, *n};

	if (auto n = consume(overflow_))
		return {read_status::ok, *n};

	std::array<std::byte, 8192> temp;

	auto want = std::min(
		max_bytes,
		temp.size()
	);

	if (remaining_)
		want = std::min(
			want,
			*remaining_
		);

	auto result = stream_.read(
		std::span<std::byte>(
			temp.data(),
			want
		)
	);

	if (result.status == stream::stream_status::closed)
		return {read_status::eof, 0};

	if (result.status != stream::stream_status::success)
		return {read_status::error, 0};

	if (result.bytes == 0)
		return {read_status::eof, 0};

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
			reinterpret_cast<char*>(temp.data() + n),
			result.bytes - n
		);
	}


	return {
		read_status::ok,
		n
	};
}