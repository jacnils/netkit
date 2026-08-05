#include <netkit/http/multipart_reader.hpp>
#include <netkit/body/multipart_part_body.hpp>
#include <cstring>
#include <string>
#include <string_view>
#include <istream>
#include <sstream>
#include <algorithm>

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

std::size_t netkit::http::utility::multipart_reader::boundary_overlap() const {
	const std::string delimiter = "\r\n--" + boundary_;

	auto max = std::min(buffer_.size(), delimiter.size() - 1);

	for (std::size_t i = max; i > 0; i--) {
		if (buffer_.compare(buffer_.size() - i, i, delimiter, 0, i) == 0) {
			return i;
		}
	}

	return 0;
}

void netkit::http::utility::multipart_reader::parse_part_headers(std::string_view headers, multipart_part& part) {
	std::istringstream stream{std::string{headers}};
	std::string line;

	while (std::getline(stream, line)) {
		if (!line.empty() && line.back() == '\r')
			line.pop_back();

		auto colon = line.find(':');

		if (colon == std::string::npos)
			continue;

		std::string key = line.substr(0, colon);
		std::string value = line.substr(colon + 1);

		auto trim = [](std::string& str) {
			while (!str.empty() && std::isspace(
				static_cast<unsigned char>(str.front())
			))
				str.erase(str.begin());

			while (!str.empty() && std::isspace(
				static_cast<unsigned char>(str.back())
			))
				str.pop_back();
		};

		auto lowercase = [](std::string& str) {
			std::ranges::transform(str, str.begin(), [](unsigned char c) { return std::tolower(c); });
		};

		trim(key);
		trim(value);
		lowercase(key);

		if (key == "content-disposition") {
			auto parse_parameter = [&](std::string_view name) -> std::string {
				auto pos = value.find(name);

				if (pos == std::string::npos)
					return {};

				pos += name.size();

				auto end = value.find('"', pos);

				if (end == std::string::npos)
					return {};

				return value.substr(pos, end - pos);
			};

			part.name = parse_parameter("name=\"");
			part.filename = parse_parameter("filename=\"");
		}
		else if (key == "content-type") {
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
		auto pos = buffer_.find("\r\n\r\n");

		if (pos != std::string::npos) {
			auto headers = buffer_.substr(0, pos);
			buffer_.erase(0, pos + 4);

			parse_part_headers(headers, part);
			return true;
		}

		if (!fill_buffer())
			return false;
	}
}

netkit::body::read_result netkit::http::utility::multipart_reader::read_part(char* out, std::size_t max_bytes) noexcept {
	if (part_finished_)
		return {body::read_status::eof, 0};

	const std::string delimiter = "\r\n--" + boundary_;

	while (true) {
		auto pos = buffer_.find(delimiter);

		if (pos != std::string::npos) {
			auto amount = std::min(pos, max_bytes);

			if (amount > 0) {
				std::memcpy(
					out,
					buffer_.data(),
					amount
				);

				buffer_.erase(0, amount);

				return {
					body::read_status::ok,
					amount
				};
			}

			part_finished_ = true;

			return {
				body::read_status::eof,
				0
			};
		}

		auto keep = boundary_overlap();

		if (buffer_.size() > keep) {
			auto amount = std::min(
				buffer_.size() - keep,
				max_bytes
			);

			std::memcpy(
				out,
				buffer_.data(),
				amount
			);

			buffer_.erase(0, amount);

			return {
				body::read_status::ok,
				amount
			};
		}

		if (!fill_buffer())
			return {
				body::read_status::error,
				0
			};
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