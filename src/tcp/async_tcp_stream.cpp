#include <netkit/tcp/async_tcp_stream.hpp>

netkit::tcp::async_tcp_stream::~async_tcp_stream() {
	this->async_tcp_stream::close();
}

netkit::io::task<void> netkit::tcp::async_tcp_stream::connect() const {
	co_await stream_.connect();
}

netkit::io::task<netkit::stream::stream_result>
netkit::tcp::async_tcp_stream::read(std::span<std::byte> buffer) {
	co_return co_await stream_.read(buffer);
}

netkit::io::task<netkit::stream::stream_result>
netkit::tcp::async_tcp_stream::write(std::span<const std::byte> buffer) {
	co_return co_await stream_.write(buffer);
}

void netkit::tcp::async_tcp_stream::close() noexcept {
	stream_.close();
}

bool netkit::tcp::async_tcp_stream::is_open() const noexcept {
	return stream_.is_open();
}

netkit::socket::addr netkit::tcp::async_tcp_stream::peer() const {
	return stream_.peer();
}

netkit::stream::async_socket_stream& netkit::tcp::async_tcp_stream::stream() {
	return stream_;
}