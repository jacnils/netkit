#include <iostream>
#include <fstream>
#include <string>
#include <netkit/netkit.hpp>

netkit::io::task<void> request(netkit::io::io_context& ctx) {
	netkit::sock::addr addr{"google.com", 80, netkit::sock::addr_type::hostname};

	netkit::sock::async_sock sock{ctx, addr, netkit::sock::type::tcp};

	co_await sock.connect();

	constexpr std::string_view request =
		"GET / HTTP/1.1\r\n"
		"Host: google.com\r\n"
		"Connection: close\r\n"
		"\r\n";

	co_await sock.send(
		request.data(),
		request.size()
	);

	std::string response;

	char buffer[8192];
	while (true) {
		auto received = co_await sock.recv(buffer, sizeof(buffer));

		if (received == 0)
			break;

		response.append(
			buffer,
			received
		);
	}

	sock.close();

	std::ofstream file("response.txt");

	if (!file) {
		std::cerr << "Failed to open file\n";
		co_return;
	}

	file << response;

	std::cout << "Response written to response.txt\n";
}

int main() {
	netkit::io::io_context ctx;
	ctx.spawn(request(ctx));
	ctx.run();
}
