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
#include <netkit/http/async_client.hpp>
#include <netkit/io/io_context.hpp>

netkit::io::task<void> make_request(netkit::io::io_context& ctx) {
    netkit::socket::addr addr{"www.google.com", 443, netkit::socket::addr_type::hostname};
    netkit::http::async_client client(ctx, addr, netkit::http::scheme::https);

    auto response = co_await client.get("/");
    auto read_body = co_await response.body->read_all();

    std::cout << "HTTP/1.1 " + std::to_string(response.status_code) + " " + std::string(netkit::http::get_message(response.status_code).value()) << std::endl;

    for (auto& it : response.headers) {
        std::cout << it.name.value() << ": " << it.value << std::endl;
    }

    std::cout << read_body << std::endl;
}

int main() {
    netkit::io::io_context ctx;


    ctx.spawn(netkit::io::timeout(make_request(ctx), std::chrono::seconds(5), []() {
        std::cerr << "Request timeout.\n";
        std::exit(EXIT_FAILURE);
    }));

    ctx.run_until_idle();

    return 0;
}
