#pragma once

#include <netkit/body/basic_async_body.hpp>

#include <string>
#include <cstring>

namespace netkit::body {
	class NETKIT_API async_buffer_body : public basic_async_body {
	public:
		async_buffer_body() = default;

		explicit async_buffer_body(std::string data) : data_(std::move(data)) {};
		netkit::io::task<read_result> read(char* buffer, std::size_t max_bytes) noexcept override {
			if (offset_ >= data_.size()) {
				co_return read_result{ read_status::eof, 0 };
			}

			std::size_t remaining = data_.size() - offset_;
			std::size_t to_read = std::min(max_bytes, remaining);

			std::memcpy(buffer, data_.data() + offset_, to_read);
			offset_ += to_read;

			co_return read_result { read_status::ok, to_read };
		}

		std::optional<std::size_t> size() const override {
			return data_.size();
		}

		bool rewind() override {
			offset_ = 0;
			return true;
		}

		const std::string& data() const { return data_; };
		void set(std::string data) {
			data_ = std::move(data);
			offset_ = 0;
		}

		void append(const std::string& data) {
			data_ += data;
		}
	private:
		std::string data_{};
		std::size_t offset_{};
	};
}