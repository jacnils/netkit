/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file main.c
 *  @license MIT
 *  @note Example code using the Netkit library.
 *  @brief A simple HTTP server that responds with "Hello, World!" to any request.
 */
#include <iostream>
#include <netkit/http/server_predefined.hpp>
#include <netkit/http/sync_server.hpp>

int main() {
    constexpr int port = 8081;
    std::cout << "Starting HTTP server on port " << port << "...\n";
    netkit::http::server::sync_server server(
        netkit::http::server::server_settings{
            .port = port,
            .enable_session = false,
            .session_directory = "./sessions",
            .session_cookie_name = "netkit-test",
            .trust_x_forwarded_for = false,
        },
        [](const netkit::http::server::request& req) -> netkit::http::server::response {
            netkit::http::server::response res;
            res.http_status = 200;
            res.body = "<html><body>Hello, World!</body></html>";
            res.content_type = "text/html";
            res.headers.push_back({"X-Test-Header", "TestValue"});

            std::cout << "Received request from: " << req.ip_address << "\n"
                      << "Endpoint: " << req.endpoint << "\n"
                      << "Method: " << req.method << "\n"
                      << "User-Agent: " << req.user_agent << "\n"
                      << "Body: " << req.body << "\n";
            return res;
        });

    std::cout << "Server started on port 8080" << ".\n"
              << "Press Ctrl+C to stop the server.\n";

    server.run();

    return 0;
}
