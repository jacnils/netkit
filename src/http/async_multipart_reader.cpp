#include <netkit/http/async_multipart_reader.hpp>
#include <netkit/body/async_multipart_part_body.hpp>
#include <cstring>
#include <string>
#include <string_view>
#include <istream>
#include <sstream>
#include <algorithm>

netkit::io::task<bool> netkit::http::utility::async_multipart_reader::fill_buffer() {
	char temp[8192];

	auto res = co_await source_.read(temp, sizeof(temp));

	if (res.get_status() == body::read_status::eof)
		co_return false;

	if (res.get_status() != body::read_status::ok)
		co_return false;

	buffer_.append(temp, res.get_bytes_read());

	co_return true;
}

void netkit::http::utility::async_multipart_reader::parse_part_headers(std::string_view headers, async_multipart_part& part) {
	std::istringstream stream{std::string{headers}};
	std::string line;

	while (std::getline(stream, line)) {
		if (!line.empty() && line.back() == '\r')
			line.pop_back();

		if (line.starts_with("Content-Disposition:")) {
			auto name_pos = line.find("name=\"");

			if (name_pos != std::string::npos) {
				name_pos += 6;

				auto end_pos = line.find('"', name_pos);

				if (end_pos != std::string::npos) {
					part.name = line.substr(
						name_pos,
						end_pos - name_pos
					);
				}
			}

			auto file_pos = line.find("filename=\"");

			if (file_pos != std::string::npos) {
				file_pos += 10;

				auto end_pos = line.find('"', file_pos);

				if (end_pos != std::string::npos) {
					part.filename = line.substr(
						file_pos,
						end_pos - file_pos
					);
				}
			}
		} else if (line.starts_with("Content-Type:")) {
			constexpr std::string_view prefix = "Content-Type:";

			auto value = std::string_view(line).substr(prefix.size());

			while (!value.empty() && value.front() == ' ')
				value.remove_prefix(1);

			part.content_type = value;
		}
	}
}

netkit::io::task<bool> netkit::http::utility::async_multipart_reader::read_boundary() {
	std::string marker = "--" + boundary_;

	while (true) {
		auto pos = buffer_.find(marker);

		if (pos == std::string::npos) {
			if (!co_await fill_buffer())
				co_return false;

			continue;
		}

		buffer_.erase(0, pos + marker.size());

		if (buffer_.starts_with("--")) {
			state_ = multipart_state::finished;
			co_return false;
		}

		if (buffer_.starts_with("\r\n"))
			buffer_.erase(0, 2);

		state_ = multipart_state::headers;
		co_return true;
	}
}

netkit::io::task<bool> netkit::http::utility::async_multipart_reader::read_headers(async_multipart_part& part) {
	while (true) {
		size_t pos = buffer_.find("\r\n\r\n");

		size_t skip = 4;

		if (pos == std::string::npos) {
			pos = buffer_.find("\n\n");
			skip = 2;
		}

		if (pos != std::string::npos) {
			std::string headers = buffer_.substr(0, pos);
			buffer_.erase(0, pos + skip);

			parse_part_headers(headers, part);
			co_return true;
		}

		if (!co_await fill_buffer())
			co_return false;
	}
}

netkit::io::task<netkit::body::read_result> netkit::http::utility::async_multipart_reader::read_part(char* out, std::size_t max_bytes) noexcept {
	if (part_finished_) {
		co_return body::read_result { body::read_status::eof, 0 };
	}

	if (state_ != multipart_state::data)
		co_return body::read_result { body::read_status::error, 0 };

	const std::string delimiter = "\r\n--" + boundary_;

	while (true) {
		auto pos = buffer_.find(delimiter);

		if (pos != std::string::npos) {
			std::size_t amount = std::min(pos, max_bytes);

			if (amount > 0) {
				std::memcpy(out, buffer_.data(), amount);
				buffer_.erase(0, amount);

				co_return body::read_result {
					body::read_status::ok,
					amount
				};
			}

			part_finished_ = true;

			co_return body::read_result {
				body::read_status::eof,
				0
			};
		}

		char temp[8192];

		auto res = co_await source_.read(temp, sizeof(temp));

		if (res.get_status() != body::read_status::ok)
			co_return res;

		buffer_.append(temp, res.get_bytes_read());
	}
}

netkit::io::task<bool> netkit::http::utility::async_multipart_reader::next(async_multipart_part& part) {
	if (state_ == multipart_state::finished)
		co_return false;

	if (state_ == multipart_state::data) {
		if (!part_finished_)
			co_return false;

		state_ = multipart_state::boundary;
	}

	if (state_ == multipart_state::boundary) {
		if (!co_await read_boundary())
			co_return false;
	}

	if (state_ == multipart_state::headers) {
		part = {};

		if (!co_await read_headers(part))
			co_return false;

		part.data = std::make_unique<body::async_multipart_part_body>(*this);

		part_finished_ = false;
		state_ = multipart_state::data;

		co_return true;
	}

	co_return false;
}