/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file main.c
 *  @license MIT
 *  @note Example code using the Netkit library.
 *  @note If netkit was built with OpenSSL support, HTTPS requests will be made.
 *  @brief A simple example demonstrating the usage of http::client::sync_client() to make a simple HTTP request.
 */
#include <iostream>
#include <fstream>
#include <netkit/http/sync_client.hpp>

void thin_http_abstraction() {
    auto http_abstr = netkit::http::client::sync_client("www.google.com", "/", 443,
                                         netkit::http::method::GET, netkit::http::version::HTTP_1_1);

    http_abstr.set_connection("Close");
    http_abstr.set_user_agent("netkit-client/1.0");
    http_abstr.set_header("Accept", "application/json");

    const auto& ref = http_abstr.get();
    for (const auto& it : ref.headers) {
        std::cerr << it.first << ": " << it.second << std::endl;
    }
    std::cout << ref.body << std::endl;
}

int main() {
    thin_http_abstraction();
}
