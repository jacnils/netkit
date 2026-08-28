/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file main.cpp
 *  @license MIT
 *  @note Example code using the Netkit library.
 *  @note See examples/socket_ssl for a TLS/SSL version of this example.
 */
#include <iostream>
#include <fstream>
#include <string_view>
#include <netkit/tcp/tcp_stream.hpp>

int main() {
	netkit::socket::addr addr{"google.com", 80, netkit::socket::addr_type::hostname};
	netkit::tcp::tcp_stream connector{addr};

	connector.connect();

    constexpr std::string_view request = "GET / HTTP/1.1\r\nHost: google.com\r\nConnection: close\r\n\r\n";

    auto write_result = connector.write_all(request);

	if (write_result.status != netkit::stream::stream_status::success) {
		throw std::runtime_error{"write failed"};
	}

	auto response = connector.read_all_string();

	connector.close();

    std::ofstream file("response.txt");

    if (file.is_open()) {
    	file.write(response.data(), response.size());
        file.close();
    } else {
        std::cerr << "Failed to open file" << std::endl;
    }

    std::cout << "Response written to response.txt" << std::endl;
}