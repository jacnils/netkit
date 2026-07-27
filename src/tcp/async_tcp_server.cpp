#include <netkit/socket/native/native_async_listener.hpp>
#include <netkit/tcp/async_tcp_server.hpp>
#include <netkit/tcp/async_tcp_stream.hpp>

netkit::tcp::async_tcp_server::async_tcp_server(io::io_context& ctx, netkit::sock::addr addr)
	: addr_(std::move(addr)), listener_(std::make_unique<sock::native::native_async_listener>(ctx, addr_, sock::type::tcp)) {}

netkit::tcp::async_tcp_server::~async_tcp_server() {
	this->close();
}

void netkit::tcp::async_tcp_server::bind() {
	listener_->bind();
}

void netkit::tcp::async_tcp_server::listen(int backlog) {
	listener_->listen(backlog);
}

void netkit::tcp::async_tcp_server::listen() {
	listener_->listen(-1);
}

netkit::io::task<std::unique_ptr<netkit::tcp::async_tcp_stream>>
netkit::tcp::async_tcp_server::accept() {
	auto socket = co_await listener_->accept();
	co_return std::make_unique<async_tcp_stream>(std::move(socket));
}

void netkit::tcp::async_tcp_server::close() noexcept {
	if (listener_)
		listener_->close();
}

const netkit::sock::addr& netkit::tcp::async_tcp_server::get_local_endpoint() const noexcept {
	return listener_->get_local_endpoint();
}