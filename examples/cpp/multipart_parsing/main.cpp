/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file main.cpp
 *  @license MIT
 *  @note Example code using the Netkit library.
 *  @brief Example demonstrating parsing multipart
 */
#include <iostream>
#include <fstream>
#include <netkit/netkit.hpp>

struct request {
	std::unordered_map<std::string, std::string> headers;
	std::unique_ptr<netkit::body::basic_body> body;
};

int main() {
	std::string boundary = "----netkit-boundary";

	std::string multipart =
		"------netkit-boundary\r\n"
		"Content-Disposition: form-data; name=\"username\"\r\n"
		"\r\n"
		"jacob\r\n"

		"------netkit-boundary\r\n"
		"Content-Disposition: form-data; name=\"file\"; filename=\"test.txt\"\r\n"
		"Content-Type: text/plain\r\n"
		"\r\n"
		"Hello from a multipart upload!\r\n"

		"------netkit-boundary--\r\n";


	/* In this example, we are simply making a fake HTTP request ourselves.  */
	request req;
	req.headers["Content-Type"] = "multipart/form-data; boundary=" + boundary;
	req.body = std::make_unique<netkit::body::buffer_body>(multipart);


	/* If the boundary is not known ahead of time, which it most of the time isn't */
	if (req.headers.find("Content-Type") != req.headers.end()) {
		std::string content_type = req.headers.at("Content-Type");
		boundary = netkit::http::utility::extract_boundary(content_type);
	}

	/* http::server::request can be used just as well as our fake request struct here. */

	netkit::http::utility::multipart_reader reader{*req.body, boundary};

	netkit::http::utility::multipart_part part;

	while (reader.next(part)) {
		char buffer[8192];

		while (true) {
			/* When calling data->read(), we are actively reading from a stream.
			 * This means, we can't call this function again later and expect the same output.
			 * In this case we're reading to a small buffer, so that we don't have to store the entire multipart in memory.
			 * If you are dealing with small requests, you can simply call read_all() to read it all into memory.
			 */
			auto result = part.data->read(buffer, sizeof(buffer));

			if (result.get_bytes_read() > 0) {
				std::cout << "Filename: " << part.filename << "\n";
				std::cout << "Content-Type: " << part.content_type << "\n";
				std::cout << "Name: " << part.name << "\n";
				std::cout.write(buffer, result.get_bytes_read());
			}

			if (result.get_status() == netkit::body::read_status::eof)
				break;

			if (result.get_status() == netkit::body::read_status::error) {
				throw std::runtime_error{"encountered an error"};
			}
		}

		std::cout << "\n";
	}
}
