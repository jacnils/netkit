#include <iostream>
#include <fstream>
#include <string>
#include <netkit/netkit.hpp>

netkit::io::task<>
handle_client(netkit::io::io_context& ctx, std::unique_ptr<netkit::sock::basic_async_sock> client) {
	char buffer[8192];

	while (true) {
		auto received = co_await client->recv(buffer, sizeof(buffer));

		if (received == 0) {
			std::cout << "client disconnected\n";
			break;
		}

		std::cout << buffer << "\n";

		co_await client->send(buffer, received);

		client->close();
	}
}

netkit::io::task<>
request(netkit::io::io_context& ctx) {
	netkit::sock::addr addr{
		"localhost",
		1337,
		netkit::sock::addr_type::hostname
	};

	netkit::sock::async_sock sock{
		ctx,
		addr,
		netkit::sock::type::tcp,
		netkit::sock::opt::reuse_addr | netkit::sock::opt::no_delay | netkit::sock::opt::no_blocking
	};

	sock.bind();
	sock.listen();

	// ReSharper disable once CppDFAEndlessLoop
	while (true) {
		auto client = co_await sock.accept();
		ctx.spawn(handle_client(ctx, std::move(client)));
	}
}

int main() {
	netkit::io::io_context ctx;
	ctx.spawn(request(ctx));
	ctx.run();
}
