/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file main.cpp
 *  @license MIT
 *  @note Example code using the Netkit library.
 *  @brief Slightly more advanced/useful HTTP file server that serves files from a specified index file and directory.
 */
#include <iostream>
#include <filesystem>
#include <netkit/netkit.hpp>

struct server_settings {
    int port{8080};
    std::string index_file{"./index.html"};
};

int main(int argc, char** argv) {
	server_settings settings{};
	std::vector<std::string> args(argv, argv + argc);
	for (int i{0}; i < args.size(); ++i) {
	    if (args[i] == "--port" && i + 1 < args.size()) {
            settings.port = std::stoi(args[i + 1]);
            ++i;
        } else if (args[i] == "--index-file" && i + 1 < args.size()) {
            settings.index_file = args[i + 1];
            ++i;
        } else if (args[i] == "--help" || args[i] == "-h") {
            std::cout << "Usage: " << args[0] << " [--port <port>] [--index-file <file>] [--help|-h]\n"
                      << "  --port <port>         Specify the port to run the HTTP server on (default: 80)\n"
                      << "  --index-file <file>   Specify the path to the index HTML file (default: ./index.html)\n"
                      << "  --help, -h            Show this help message\n";
            return 0;
        }
	}

    std::cout << "Starting HTTP server on port " << settings.port << "...\n";
    netkit::http::server::sync_server server(
        netkit::http::server::server_settings{
            .port = settings.port,
            .enable_session = false,
            .session_directory = "./sessions",
            .session_cookie_name = "netkit-server",
            .trust_x_forwarded_for = false,
        },
        [&settings](const netkit::http::server::request& req) -> netkit::http::server::response {
            netkit::http::server::response res;
			std::string parent_path = std::filesystem::path(settings.index_file).parent_path().string();
			if (!std::filesystem::is_directory(parent_path)) {
				parent_path = ".";
			}

        	if (req.endpoint.find("..") != std::string::npos) {
				res.http_status = 403;
				res.body = "<html><body><h1>403 Forbidden</h1></body></html>";
				res.content_type = "text/html";
				res.headers.push_back({"X-Server", "netkit-http-server/1.0"});
				return res;
			}

			if ((req.endpoint == "/" || req.endpoint.empty()) && std::filesystem::is_regular_file(settings.index_file)) {
				res.http_status = 200;
				res.body = netkit::utility::read_file(settings.index_file);
				res.content_type = netkit::utility::get_appropriate_content_type(settings.index_file);
				res.headers.push_back({"X-Server", "netkit-http-server/1.0"});
			} else if (std::filesystem::is_regular_file(std::filesystem::path(parent_path) / req.endpoint.substr(1))) {
				std::string file_path = std::filesystem::path(parent_path) / req.endpoint.substr(1);
				res.http_status = 200;
				res.body = netkit::utility::read_file(file_path);
				res.content_type = netkit::utility::get_appropriate_content_type(file_path);
				res.headers.push_back({"X-Server", "netkit-http-server/1.0"});
				res.headers.push_back({"Content-Disposition", "inline"});
			} else {
				res.http_status = 404;
				res.body = "<html><body><h1>404 Not Found</h1></body></html>";
				res.content_type = "text/html";
				res.headers.push_back({"X-Server", "netkit-http-server/1.0"});
			}

			std::cout << "Received request from: " << req.ip_address << "\n"
					  << "Endpoint: " << req.endpoint << "\n"
					  << "Method: " << req.method << "\n"
					  << "User-Agent: " << req.user_agent << "\n"
					  << "Body: " << req.body << "\n";

			return res;
        });

    std::cout << "Server started on port " << settings.port << ".\n"
              << "Press Ctrl+C to stop the server.\n";

    server.run();
}
