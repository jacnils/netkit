#include <netkit/http/multipart_reader.hpp>
#include <netkit/body/multipart_part_body.hpp>
#include <cstring>
#include <string>
#include <string_view>

bool netkit::http::utility::multipart_reader::fill_buffer() {
	char temp[8192];

	auto res = source_.read(temp, sizeof(temp));

	if (res.get_status() == body::read_status::eof)
		return false;

	if (res.get_status() != body::read_status::ok)
		return false;

	buffer_.append(temp, res.get_bytes_read());

	return true;
}

void netkit::http::utility::multipart_reader::parse_part_headers(std::string_view headers, multipart_part& part) {
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

bool netkit::http::utility::multipart_reader::read_boundary() {
	std::string marker = "--" + boundary_;

	while (true) {
		auto pos = buffer_.find(marker);

		if (pos == std::string::npos) {
			if (!fill_buffer())
				return false;

			continue;
		}

		buffer_.erase(0, pos + marker.size());

		if (buffer_.starts_with("--")) {
			state_ = multipart_state::finished;
			return false;
		}

		if (buffer_.starts_with("\r\n"))
			buffer_.erase(0, 2);

		state_ = multipart_state::headers;
		return true;
	}
}

bool netkit::http::utility::multipart_reader::read_headers(multipart_part& part) {
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
			return true;
		}

		if (!fill_buffer())
			return false;
	}
}

netkit::body::read_result netkit::http::utility::multipart_reader::read_part(char* out, std::size_t max_bytes) noexcept {
	if (part_finished_) {
		return { body::read_status::eof, 0 };
	}

	if (state_ != multipart_state::data)
		return { body::read_status::error, 0 };

	const std::string delimiter = "\r\n--" + boundary_;

	while (true) {
		auto pos = buffer_.find(delimiter);

		if (pos != std::string::npos) {
			std::size_t amount = std::min(pos, max_bytes);

			if (amount > 0) {
				std::memcpy(out, buffer_.data(), amount);
				buffer_.erase(0, amount);

				return {
					body::read_status::ok,
					amount
				};
			}

			if (amount == 0) {
				part_finished_ = true;

				return {
					body::read_status::eof,
					0
				};
			}

			part_finished_ = true;

			return {
				body::read_status::eof,
				0
			};
		}

		char temp[8192];

		auto res = source_.read(temp, sizeof(temp));

		if (res.get_status() != body::read_status::ok)
			return res;

		buffer_.append(temp, res.get_bytes_read());
	}
}

netkit::body::read_result netkit::http::utility::multipart_reader::read_current_part(char* out, size_t max) noexcept {
	std::string delimiter =
		"\r\n--" + boundary_;

	while (true) {
		auto pos = buffer_.find(delimiter);

		if (pos != std::string::npos) {
			size_t amount = std::min(pos, max);

			memcpy(
				out,
				buffer_.data(),
				amount
			);

			buffer_.erase(0, amount);

			if (amount == pos) {
				state_ = multipart_state::boundary;
			}

			return {
				body::read_status::ok,
				amount
			};
		}

		if (!fill_buffer())
			return { body::read_status::eof, 0 };
	}
}

bool netkit::http::utility::multipart_reader::next(multipart_part& part) {
	if (state_ == multipart_state::finished)
		return false;

	if (state_ == multipart_state::data) {
		if (!part_finished_)
			return false;

		state_ = multipart_state::boundary;
	}

	if (state_ == multipart_state::boundary) {
		if (!read_boundary())
			return false;
	}

	if (state_ == multipart_state::headers) {
		part = {};

		if (!read_headers(part))
			return false;

		part.data = std::make_unique<body::multipart_part_body>(*this);

		part_finished_ = false;
		state_ = multipart_state::data;

		return true;
	}

	return false;
}