#include <netkit/uds/uds_server.hpp>
#include <netkit/uds/uds_stream.hpp>

#ifndef NETKIT_DKP

netkit::uds::uds_server::uds_server(netkit::socket::addr addr)
	: addr_(std::move(addr)), listener_(std::make_unique<socket::native::native_sync_listener>(addr_, socket::type::uds)) {}

netkit::uds::uds_server::~uds_server() {
	this->close();
}

void netkit::uds::uds_server::bind() {
	listener_->bind();
}

void netkit::uds::uds_server::listen(int backlog) {
	listener_->listen(backlog);
}

void netkit::uds::uds_server::listen() {
	listener_->listen(-1);
}

std::unique_ptr<netkit::uds::uds_stream>
netkit::uds::uds_server::accept() {
	auto socket = listener_->accept();

	return std::make_unique<netkit::uds::uds_stream>(
		std::move(socket)
	);
}

void netkit::uds::uds_server::close() noexcept {
	if (listener_)
		listener_->close();
}

const netkit::socket::addr& netkit::uds::uds_server::get_local_endpoint() const noexcept {
	return listener_->get_local_endpoint();
}

#endif