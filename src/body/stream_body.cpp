#include <algorithm>
#define NOMINMAX // some windows shit
#include <netkit/body/stream_body.hpp>

netkit::body::read_result netkit::body::stream_body::read(char* out, std::size_t max_bytes) noexcept {
	if (max_bytes == 0)
		return {read_status::ok, 0};

	if (remaining_ && *remaining_ == 0)
		return {read_status::eof, 0};

	std::size_t total = 0;

	if (!buffer_.empty()) {
		std::size_t n = std::min({
			max_bytes,
			buffer_.size(),
			remaining_.value_or(buffer_.size())
		});

		std::memcpy(out, buffer_.data(), n);

		buffer_.erase(0, n);

		if (remaining_)
			*remaining_ -= n;

		return {read_status::ok, n};
	}

	if (!overflow_.empty()) {
		std::size_t n = std::min({
			max_bytes,
			overflow_.size(),
			remaining_.value_or(overflow_.size())
		});

		std::memcpy(out, overflow_.data(), n);

		overflow_.erase(0, n); // could optimize later
		total += n;

		if (remaining_)
			*remaining_ -= n;

		return {read_status::ok, n};
	}


	std::size_t want = max_bytes;

	if (remaining_)
		want = std::min(want, *remaining_);


	auto result = socket_.recv(3, "", want);

	if (result.status == sock::recv_status::closed)
		return {read_status::eof, 0};

	if (result.status == sock::recv_status::timeout)
		return {read_status::timeout, 0};

	if (result.status != sock::recv_status::success)
		return {read_status::error, 0};

	if (result.data.empty())
		return {read_status::timeout, 0};


	std::size_t n = std::min(max_bytes, result.data.size());

	std::memcpy(out, result.data.data(), n);

	if (remaining_)
		*remaining_ -= n;


	if (n < result.data.size()) {
		overflow_.assign(
			result.data.data() + n,
			result.data.size() - n
		);
	}


	return {read_status::ok, n};
}