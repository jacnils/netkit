#pragma once

#include <netkit/body/basic_body.hpp>

#include <string_view>
#include <cstring>
#include <ranges>

namespace netkit::body {
	class NETKIT_API buffer_body_view : public basic_body {
	public:
		buffer_body_view() = default;

		explicit buffer_body_view(std::string_view data) : data_(std::move(data)) {};
		read_result read(char* buffer, std::size_t max_bytes) noexcept override {
			if (offset_ >= data_.size()) {
				return { read_status::eof, 0 };
			}

			std::size_t remaining = data_.size() - offset_;
			std::size_t to_read = std::min(max_bytes, remaining);

			std::memcpy(buffer, data_.data() + offset_, to_read);
			offset_ += to_read;

			return { read_status::ok, to_read };
		}

		std::optional<std::size_t> size() const override {
			return data_.size();
		}

		bool rewind() override {
			offset_ = 0;
			return true;
		}

		const std::string_view& data() const { return data_; };
		void set(std::string_view data) {
			data_ = std::move(data);
			offset_ = 0;
		}
	private:
		std::string_view data_{};
		std::size_t offset_{};
	};
}