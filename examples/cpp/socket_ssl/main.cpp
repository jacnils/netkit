/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file main.c
 *  @license MIT
 *  @note Example code using the Netkit library.
 *  @note Only functional if Netkit was built with OpenSSL support.
 *  @note See examples/socket/main.c for a non-SSL/TLS version.
 *  @brief A lower-level example demonstrating the usage of sync_sock to make a simple HTTP request, with SSL/TLS.
 */
#include <iostream>
#include <fstream>
#include <string_view>
#include <netkit/netkit.hpp>

int main() {
    netkit::sock::sock_addr addr("google.com", 443, netkit::sock::sock_addr_type::hostname);
    std::unique_ptr<netkit::sock::basic_sync_sock> _sock = std::make_unique<netkit::sock::sync_sock>(
        addr, netkit::sock::sock_type::tcp);

    netkit::sock::ssl_sync_sock sock((std::move(_sock)),
        netkit::sock::mode::client,
        netkit::sock::version::TLS_1_2,
        netkit::sock::verification::peer
        );

    sock.connect();
    sock.perform_handshake();

    constexpr std::string_view request = "GET / HTTP/1.1\r\nHost: google.com\r\nConnection: close\r\n\r\n";
    sock.send(request.data());
    std::string response = sock.recv(-1).data;
    sock.close();

    std::ofstream file("response.txt");
    if (file.is_open()) {
        file << response;
        file.close();
    } else {
        std::cerr << "Failed to open file" << std::endl;
    }
    std::cout << "Response written to response.txt" << std::endl;
}
