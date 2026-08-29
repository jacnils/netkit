/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file main.cpp
 *  @license MIT
 *  @note Example code using the Netkit library.
 */
#include <iostream>
#include <filesystem>
#include <netkit/http/client.hpp>

int main(int argc, char** argv) {
    netkit::socket::addr addr{"www.google.com", 443, netkit::socket::addr_type::hostname};
    netkit::http::client client(addr, netkit::http::scheme::https);

    auto response = client.get("/");
    auto read_body = response.body->read_all();

    std::cout << "HTTP/1.1 " + std::to_string(response.status_code) + " " + std::string(netkit::http::get_message(response.status_code).value()) << std::endl;

    for (auto& it : response.headers) {
        std::cout << it.name.value() << ": " << it.value << std::endl;
    }

    std::cout << read_body << std::endl;
}