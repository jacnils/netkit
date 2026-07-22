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
#include <netkit/body/buffer_body.hpp>

int main() {
	std::string test_string = "this_is_a_test_string";
	netkit::body::buffer_body body(test_string);

	std::size_t read = 0;

	while (read < test_string.size()) {
		char s[4];
		body.read(s, 3);

		std::cout << s << "\n";

		read += strlen(s);
	}

	return 0;
}
