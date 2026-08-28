#include <algorithm>
#include <netkit/http/multipart.hpp>
#include <netkit/body/buffer_body.hpp>
#include <sstream>
#include <string>
#include <vector>

std::string netkit::http::utility::extract_boundary(const std::string& content_type) {
	const std::string key = "boundary=";

	auto pos = content_type.find(key);
	if (pos == std::string::npos)
		return {};

	std::string boundary = content_type.substr(pos + key.size());

	while (!boundary.empty() && std::isspace(static_cast<unsigned char>(boundary.back())))
		boundary.pop_back();

	if (boundary.size() >= 2 &&
		boundary.front() == '"' &&
		boundary.back() == '"') {
		boundary = boundary.substr(1, boundary.size() - 2);
		}

	return boundary;
}

std::vector<netkit::http::utility::multipart_part> netkit::http::utility::parse_multipart_form_data(const char* body, std::size_t size, const std::string& content_type) {
	std::vector<multipart_part> parts;

	std::string boundary = extract_boundary(content_type);
	if (boundary.empty())
		return parts;

	std::string full_boundary = "--" + boundary;
	std::string end_boundary  = full_boundary + "--";

	const char* ptr = body;
	const char* end = body + size;

	auto find_boundary = [&](const char* start) -> const char* {
		return std::search(start, end, full_boundary.begin(), full_boundary.end());
	};

	const char* cur = find_boundary(ptr);
	if (cur == end)
		return parts;

	cur += full_boundary.size();

	while (cur < end) {
		if (cur + 2 <= end && cur[0] == '\r' && cur[1] == '\n')
			cur += 2;

		if (std::search(cur, end, end_boundary.begin(), end_boundary.end()) == cur)
			break;

		constexpr std::string_view delimiter = "\r\n\r\n";
		const char* header_end = std::search(cur, end, delimiter.begin(), delimiter.end());

		if (header_end == end)
			break;

		std::string headers_block(cur, header_end);
		cur = header_end + 4;

		multipart_part part;

		std::istringstream stream(headers_block);
		std::string		   line;

		while (std::getline(stream, line)) {
			if (!line.empty() && line.back() == '\r')
				line.pop_back();

			if (line.starts_with("Content-Disposition:")) {
				auto name_pos = line.find("name=\"");
				if (name_pos != std::string::npos) {
					name_pos += 6;
					auto end_pos = line.find('"', name_pos);
					part.name	 = line.substr(name_pos, end_pos - name_pos);
				}

				auto file_pos = line.find("filename=\"");
				if (file_pos != std::string::npos) {
					file_pos += 10;
					auto end_pos  = line.find('"', file_pos);
					part.filename = line.substr(file_pos, end_pos - file_pos);
				}
			}

			if (line.starts_with("Content-Type:")) {
				part.content_type = line.substr(13);
				if (!part.content_type.empty() && part.content_type[0] == ' ')
					part.content_type.erase(0, 1);
			}
		}

		const char* next_boundary = find_boundary(cur);
		if (next_boundary == end)
			break;

		const char* data_end = next_boundary;
		if (data_end - body >= 2 && data_end[-2] == '\r' && data_end[-1] == '\n') {
			data_end -= 2;
		}

		auto buf_body = std::make_unique<netkit::body::buffer_body>();

		std::string tmp{};
		tmp.assign(cur, data_end);

		buf_body->set(std::move(tmp));

		part.data = std::move(buf_body);

		parts.push_back(std::move(part));

		cur = next_boundary + full_boundary.size();
	}

	return parts;
}

std::vector<netkit::http::utility::multipart_part> netkit::http::utility::parse_multipart_form_data(body::basic_body& body, const std::string& content_type) {
	auto data = body.read_all();

	return netkit::http::utility::parse_multipart_form_data(
		data.data(),
		data.size(),
		content_type
	);
}