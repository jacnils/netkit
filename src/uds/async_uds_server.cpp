#include <netkit/socket/native/native_async_listener.hpp>
#include <netkit/uds/async_uds_server.hpp>
#include <netkit/uds/async_uds_stream.hpp>

#ifndef NETKIT_DKP

netkit::uds::async_uds_server::async_uds_server(io::io_context& ctx, netkit::sock::addr addr)
	: addr_(std::move(addr)), listener_(std::make_unique<sock::native::native_async_listener>(ctx, addr_, sock::type::uds)) {}

netkit::uds::async_uds_server::~async_uds_server() {
	this->close();
}

void netkit::uds::async_uds_server::bind() {
	listener_->bind();
}

void netkit::uds::async_uds_server::listen(int backlog) {
	listener_->listen(backlog);
}

void netkit::uds::async_uds_server::listen() {
	listener_->listen(-1);
}

netkit::io::task<std::unique_ptr<netkit::uds::async_uds_stream>>
netkit::uds::async_uds_server::accept() {
	auto socket = co_await listener_->accept();
	co_return std::make_unique<async_uds_stream>(std::move(socket));
}

void netkit::uds::async_uds_server::close() noexcept {
	if (listener_)
		listener_->close();
}

const netkit::sock::addr& netkit::uds::async_uds_server::get_local_endpoint() const noexcept {
	return listener_->get_local_endpoint();
}

#endif