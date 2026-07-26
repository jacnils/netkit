#include <netkit/tcp/tcp_server.hpp>
#include <netkit/tcp/tcp_connection.hpp>

netkit::tcp::tcp_server::tcp_server(netkit::sock::addr addr)
	: addr_(std::move(addr)), listener_(std::make_unique<sock::native::native_sync_listener>(addr_, sock::type::tcp)) {}

netkit::tcp::tcp_server::~tcp_server() {
	this->close();
}

void netkit::tcp::tcp_server::bind() {
	listener_->bind();
}

void netkit::tcp::tcp_server::listen(int backlog) {
	listener_->listen(backlog);
}

void netkit::tcp::tcp_server::listen() {
	listener_->listen(-1);
}

std::unique_ptr<netkit::tcp::tcp_connection>
netkit::tcp::tcp_server::accept() {
	auto socket = listener_->accept();

	return std::make_unique<netkit::tcp::tcp_connection>(
		std::move(socket)
	);
}

void netkit::tcp::tcp_server::close() noexcept {
	if (listener_)
		listener_->close();
}

const netkit::sock::addr& netkit::tcp::tcp_server::get_local_endpoint() const noexcept {
	return listener_->get_local_endpoint();
}