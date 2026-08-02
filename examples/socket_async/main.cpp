#include <iostream>
#include <fstream>
#include <string>
#include <netkit/netkit.hpp>

netkit::io::task<>
request(netkit::io::io_context& ctx) {
	netkit::sock::addr addr{"google.com", 80, netkit::sock::addr_type::hostname};
	netkit::tcp::async_tcp_stream sock(ctx, addr);

	co_await sock.connect();

	constexpr std::string_view http_request =
		"GET / HTTP/1.1\r\n"
		"Host: google.com\r\n"
		"Connection: close\r\n"
		"\r\n";

	auto write_result = co_await sock.write_all(http_request);

	if (write_result.status != netkit::stream::stream_status::success)
		throw std::runtime_error("write failed");

	std::string response;

	std::array<std::byte, 8192> buffer{};

	while (true) {
		auto received = co_await sock.read(buffer);

		if (received.status == netkit::stream::stream_status::eof)
			break;

		if (received.status != netkit::stream::stream_status::success)
			throw std::runtime_error("read failed");

		response.append(
			reinterpret_cast<const char*>(buffer.data()),
			received.bytes
		);
	}

	sock.close();

	std::ofstream file("response.txt");

	if (!file)
		throw std::runtime_error("failed to open file");

	file << response;

	std::cout << "Response written to response.txt\n";
}

int main() {
	netkit::io::io_context ctx;

	ctx.spawn(request(ctx));
	ctx.run_until_idle();

	return 0;
}
