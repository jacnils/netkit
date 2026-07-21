/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file main.cpp
 *  @license MIT
 *  @note Example code using the Netkit library.
 *  @note Only functional if Netkit was built with OpenSSL support.
 *  @note See examples/socket/main.cpp for a non-SSL/TLS version.
 *  @brief A lower-level example demonstrating the usage of sync_sock to make a simple HTTP request, with SSL/TLS.
 */
#include <iostream>
#include <fstream>
#include <string_view>
#include <netkit/netkit.hpp>

int main() {
    netkit::sock::addr addr("google.com", 443, netkit::sock::addr_type::hostname);
    std::unique_ptr<netkit::sock::basic_sync_sock> _sock = std::make_unique<netkit::sock::sync_sock>(
        addr, netkit::sock::type::tcp);

    netkit::sock::ssl_sync_sock sock((std::move(_sock)),
        netkit::sock::mode::client,
        netkit::sock::version::TLS_1_2,
        netkit::sock::verification::peer
        );

    sock.connect();
    sock.perform_handshake();

    constexpr std::string_view request = "GET / HTTP/1.1\r\nHost: google.com\r\nConnection: close\r\n\r\n";
    std::string response;

    int sent = sock.send(request.data(), request.size());

    while (true) {
        auto res = sock.recv(6);

        response += res.data;

        if (res.status == netkit::sock::recv_status::closed)
			break;

		if (res.status == netkit::sock::recv_status::timeout)
			break;

        if (res.status == netkit::sock::recv_status::error)
			throw std::runtime_error("recv failed");
    }

    std::cout << response << std::flush;

	return EXIT_SUCCESS;
}
