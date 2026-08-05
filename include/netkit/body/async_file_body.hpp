#pragma once

#include <filesystem>
#include <fstream>
#include <netkit/body/basic_async_body.hpp>
#include <string>

namespace netkit::body {
	class NETKIT_API async_file_body : public basic_async_body {
		void initialize_stream() {
			if (file_) {
				file_.seekg(0, std::ios::end);
				size_ = static_cast<std::size_t>(file_.tellg());
				file_.seekg(0, std::ios::beg);
			}
		}
	public:
		explicit async_file_body(const std::string& path) : file_(path, std::ios::binary) {
			initialize_stream();
		}

		explicit async_file_body(const std::filesystem::path& path) : file_(path, std::ios::binary) {
			initialize_stream();
		}

		io::task<read_result> read(char* buffer, std::size_t max_bytes) noexcept override {
			if (!file_ || file_.eof()) {
				co_return read_result { read_status::eof, 0 };
			}

			file_.read(buffer, static_cast<std::streamsize>(max_bytes));
			std::size_t n = static_cast<std::size_t>(file_.gcount());

			if (n > 0) {
				co_return read_result { read_status::ok, n };
			}

			if (file_.eof()) {
				co_return read_result { read_status::eof, 0 };
			}

			co_return read_result { read_status::error, 0 };
		}

		std::optional<std::size_t> size() const override {
			return size_;
		}

		bool rewind() override {
			if (!file_) return false;
			file_.clear();
			file_.seekg(0, std::ios::beg);
			return true;
		}

		bool is_open() const {
			return file_.is_open();
		}
	private:
		std::ifstream file_;
		std::optional<std::size_t> size_;
	};
}