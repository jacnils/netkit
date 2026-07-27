#include <netkit/tcp/tcp_stream.hpp>

netkit::tcp::tcp_stream::~tcp_stream() {
	this->tcp_stream::close();
}

void netkit::tcp::tcp_stream::connect() {
	stream_.connect();
}

netkit::stream::stream_result netkit::tcp::tcp_stream::read(std::span<std::byte> buffer) {
	return stream_.read(buffer);
}

netkit::stream::stream_result netkit::tcp::tcp_stream::write(std::span<const std::byte> buffer) {
	return stream_.write(buffer);
}

void netkit::tcp::tcp_stream::close() noexcept {
	stream_.close();
}

netkit::sock::addr netkit::tcp::tcp_stream::peer() const {
	return stream_.peer();
}

netkit::stream::socket_stream& netkit::tcp::tcp_stream::stream() {
	return stream_;
}