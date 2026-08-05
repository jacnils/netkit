#include <iostream>
#include <filesystem>
#include <netkit/netkit.hpp>

struct server_settings {
    int port{8080};
    std::string index_file{"./index.html"};
};

netkit::io::task<> server(netkit::io::io_context& ctx, server_settings& settings) {
	netkit::http::server::async_server<> server(
		ctx,
        netkit::http::server::server_settings{
            .port = settings.port,
            .enable_session = false,
            .session_directory = "./sessions",
            .session_cookie_name = "netkit-server",
            .trust_x_forwarded_for = false,
        },
        [&settings](
        const netkit::http::server::async_request& req) -> netkit::io::task<netkit::http::server::async_response>
        {
            netkit::http::server::async_response res;
			std::string parent_path = std::filesystem::path(settings.index_file).parent_path().string();
			if (!std::filesystem::is_directory(parent_path)) {
				parent_path = ".";
			}

			std::string body_{};
			if (req.body) {
				body_ = co_await req.body->read_all();
			}

			if (req.endpoint.find("..") != std::string::npos) {
				res.http_status = 403;
				res.body = std::make_unique<netkit::body::async_buffer_body>("<html><body><h1>403 Forbidden</h1></body></html>");
				res.content_type = "text/html";
				res.headers.push_back({"X-Server", "netkit-http-server/1.0"});
				co_return res;
			}

			if ((req.endpoint == "/" || req.endpoint.empty()) && std::filesystem::is_regular_file(settings.index_file)) {
				res.http_status = 200;
				res.body = std::make_unique<netkit::body::async_file_body>(settings.index_file);
				res.content_type = netkit::utility::get_appropriate_content_type(settings.index_file);
				res.headers.push_back({"X-Server", "netkit-http-server/1.0"});
			} else if (std::filesystem::is_regular_file(std::filesystem::path(parent_path) / req.endpoint.substr(1))) {
				std::string file_path = (std::filesystem::path(parent_path) / req.endpoint.substr(1)).string();
				res.http_status = 200;
				res.body = std::make_unique<netkit::body::async_file_body>(file_path);
				res.content_type = netkit::utility::get_appropriate_content_type(file_path);
				res.headers.push_back({"X-Server", "netkit-http-server/1.0"});
				res.headers.push_back({"Content-Disposition", "inline"});
			} else {
				res.http_status = 404;
				res.body = std::make_unique<netkit::body::async_buffer_body>("<html><body><h1>404 Not Found</h1></body></html>");
				res.content_type = "text/html";
				res.headers.push_back({"X-Server", "netkit-http-server/1.0"});
			}

			std::cout << "Received request from: " << req.ip_address << "\n"
					  << "Endpoint: " << req.endpoint << "\n"
					  << "Method: " << req.method << "\n"
					  << "User-Agent: " << req.user_agent << "\n"
					  << "Body: " << body_ << "\n";

			co_return res;
        });

	co_await server.run();
	co_return;
}

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

	netkit::io::io_context ctx;

	ctx.spawn(server(ctx, settings));

    std::cout << "Server started on port " << settings.port << ".\n"
              << "Press Ctrl+C to stop the server.\n";

	ctx.run();
}