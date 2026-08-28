#include <iostream>
#include <fstream>
#include <string>
#include <netkit/netkit.hpp>

netkit::io::task<>
handle_client(netkit::io::io_context& ctx, std::unique_ptr<netkit::tcp::async_tcp_stream> client) {
	std::array<std::byte, 8192> buffer{};

	std::string response;
	while (true) {
		auto received = co_await client->read(buffer);

		if (received.status != netkit::stream::stream_status::success) {
			throw std::runtime_error{"error"};
		}

		if (received.bytes == 0) {
			std::cout << "client disconnected\n";
			break;
		}

		response.append(reinterpret_cast<const char*>(buffer.data()), received.bytes);

		co_await client->write_all(std::span<const std::byte>(buffer.data(), received.bytes));

		break;
	}

	std::cerr << "client " << client->peer().get_ip() << " disconnected\n";

	client->close();
}

netkit::io::task<>
request(netkit::io::io_context& ctx) {
	netkit::socket::addr addr{"localhost", 1337, netkit::socket::addr_type::hostname};

	netkit::tcp::async_tcp_server server{ctx, addr};

	server.bind();
	server.listen();

	std::cerr << "server listening on port " << addr.get_port() << "\n";

	// ReSharper disable once CppDFAEndlessLoop
	while (true) {
		auto client = co_await server.accept();
		std::cerr << "client " << client->peer().get_ip() << " connected\n";
		ctx.spawn(handle_client(ctx, std::move(client)));
	}
}

int main() {
	netkit::io::io_context ctx;
	ctx.spawn(request(ctx));
	ctx.run();
}
