#include <netkit/uds/async_uds_stream.hpp>

#ifndef NETKIT_DKP

netkit::uds::async_uds_stream::~async_uds_stream() {
	this->async_uds_stream::close();
}

netkit::io::task<void> netkit::uds::async_uds_stream::connect() const {
	co_await stream_.connect();
}

netkit::io::task<netkit::stream::stream_result>
netkit::uds::async_uds_stream::read(std::span<std::byte> buffer) {
	co_return co_await stream_.read(buffer);
}

netkit::io::task<netkit::stream::stream_result>
netkit::uds::async_uds_stream::write(std::span<const std::byte> buffer) {
	co_return co_await stream_.write(buffer);
}

void netkit::uds::async_uds_stream::close() noexcept {
	stream_.close();
}

bool netkit::uds::async_uds_stream::is_open() const noexcept {
	return stream_.is_open();
}

netkit::socket::addr netkit::uds::async_uds_stream::peer() const {
	return stream_.peer();
}

netkit::stream::async_socket_stream& netkit::uds::async_uds_stream::stream() {
	return stream_;
}

#endif