#include <netkit/body/stream_body.hpp>

netkit::body::read_result netkit::body::stream_body::read(char* buffer, std::size_t max_bytes) noexcept {
	if (remaining_ == 0) {
		return {read_status::eof, 0};
	}

	std::size_t copied = 0;

	if (!overflow_.empty()) {
		copied = std::min({
			max_bytes,
			remaining_,
			overflow_.size()
		});

		std::memcpy(buffer, overflow_.data(), copied);

		overflow_.erase(0, copied);
		remaining_ -= copied;

		return {read_status::ok, copied};
	}


	auto result = socket_.recv();

	if (result.status == sock::recv_status::closed) {
		return {read_status::eof, 0};
	}


	if (result.status != sock::recv_status::success) {
		return {read_status::error, 0};
	}


	copied = std::min({
		max_bytes,
		remaining_,
		result.data.size()
	});


	std::memcpy(
		buffer,
		result.data.data(),
		copied
	);


	remaining_ -= copied;

	if (result.data.size() > copied) {
		overflow_.assign(
			result.data.data() + copied,
			result.data.size() - copied
		);
	}

	return {read_status::ok, copied};
}