#pragma once

#include <algorithm>
#include <netkit/stream/basic_stream.hpp>
#include <span>
#include <vector>

namespace netkit::stream {

class memory_stream : public basic_stream {
public:
	memory_stream() = default;

	explicit memory_stream(std::vector<std::byte> data) : buffer_(std::move(data)) {}

	stream_result read(std::span<std::byte> buffer) override {
		if (read_pos_ >= buffer_.size()) {
			return {
				0,
				stream_status::closed
			};
		}

		auto available = buffer_.size() - read_pos_;
		auto amount = std::min(
			available,
			buffer.size()
		);

		std::copy_n(
			buffer_.data() + read_pos_,
			amount,
			buffer.data()
		);

		read_pos_ += amount;

		return {
			amount,
			stream_status::success
		};
	}

	stream_result write(std::span<const std::byte> buffer) override {
		buffer_.insert(
			buffer_.end(),
			buffer.begin(),
			buffer.end()
		);

		return {
			buffer.size(),
			stream_status::success
		};
	}

	void close() noexcept override {
		closed_ = true;
	}

	[[nodiscard]] std::span<const std::byte> data() const noexcept {
		return buffer_;
	}

private:
	std::vector<std::byte> buffer_;
	std::size_t read_pos_{0};
	bool closed_{false};
};

}