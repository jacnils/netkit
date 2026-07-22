/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file main.c
 *  @license MIT
 *  @note Example code using the Netkit library.
 *  @brief Example demonstrating buffer_body
 */
#include <iostream>
#include <fstream>
#include <string_view>
#include <netkit/body/file_body.hpp>

int main() {
	netkit::body::file_body fb{std::string("file.txt")};

	std::size_t file_size = fb.size().value();
	std::size_t read_bytes = 0;

	while (read_bytes < file_size) {
		char buf[5];
		
		auto ret = fb.read(buf, 4);

		read_bytes += ret.get_bytes_read();

		std::cout << buf << "\n";	
	}

	return 0;
}
