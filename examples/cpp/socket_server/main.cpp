#include <iostream>
#include <string_view>

#include <netkit/tcp/tcp_stream.hpp>
#include <netkit/tcp/tcp_server.hpp>

[[noreturn]] int main() {
	netkit::sock::addr addr{
		"0.0.0.0", // or simply "localhost"
		1337,
		netkit::sock::addr_type::ipv4
	};

	netkit::tcp::tcp_server server{addr};

	server.bind();
	server.listen();

	std::cout << "server listening on port 1337\n";

	while (true) {
		auto client = server.accept();

		std::cout << "client connected from " << client->peer().get_ip() << "\n";

		// we can read with client->read()
		// in a http context, you might want to read until you find \r\n\r\n
		// and then parse the headers. then you'd continue reading.

		constexpr std::string_view response =
			"HTTP/1.1 200 OK\r\n"
			"Content-Length: 12\r\n"
			"Connection: close\r\n"
			"\r\n"
			"Hello world!";

		auto result = client->write_all(response);

		if (result.status != netkit::stream::stream_status::success) {
			std::cerr << "failed to send response\n";
		}

		client->close();

		std::cout << "client closed\n";
	}


	// we will never actually reach this
	server.close();
}