/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file main.c
 *  @license MIT
 *  @note Example code using the Netkit library.
 *  @note See examples/socket_ssl for a TLS/SSL version of this example.
 *  @brief A simple example demonstrating stream_body
 */
#include <iostream>
#include <fstream>
#include <string_view>
#include <ranges>
#include <netkit/sock/addr.hpp>
#include <netkit/sock/sync_sock.hpp>
#include <netkit/body/stream_body.hpp>

int main() {
    netkit::sock::addr addr("www.google.com", 80, netkit::sock::addr_type::hostname);
    netkit::sock::sync_sock sock(addr, netkit::sock::type::tcp);

    sock.connect();

    constexpr std::string_view request = "GET / HTTP/1.1\r\nHost: www.google.com\r\nConnection: close\r\n\r\n";

    sock.send(request.data());

    netkit::body::stream_body body(sock, std::nullopt);

    while (true) {
    	char buf[4] = {0};

	auto result = body.read(buf, 3);

	if (result.get_status() == netkit::body::read_status::eof)
		break;

	buf[result.get_bytes_read()] = '\0';

	std::cout << buf << "\n";
    }

    return 0;
}
