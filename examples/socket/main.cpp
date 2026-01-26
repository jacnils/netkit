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
 *  @brief A lower-level example demonstrating the usage of sync_sock to make a simple HTTP request.
 */
#include <iostream>
#include <fstream>
#include <string_view>
#include <netkit/netkit.hpp>

int main() {
    netkit::sock::sock_addr addr("google.com", 80, netkit::sock::sock_addr_type::hostname);
    netkit::sock::sync_sock sock(addr, netkit::sock::sock_type::tcp);

    sock.connect();

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
