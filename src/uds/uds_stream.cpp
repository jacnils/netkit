#include <netkit/uds/uds_stream.hpp>

#ifndef NETKIT_DKP

netkit::uds::uds_stream::~uds_stream() {
	this->uds_stream::close();
}

void netkit::uds::uds_stream::connect() {
	stream_.connect();
}

netkit::stream::stream_result netkit::uds::uds_stream::read(std::span<std::byte> buffer) {
	return stream_.read(buffer);
}

netkit::stream::stream_result netkit::uds::uds_stream::write(std::span<const std::byte> buffer) {
	return stream_.write(buffer);
}

void netkit::uds::uds_stream::close() noexcept {
	stream_.close();
}

netkit::sock::addr netkit::uds::uds_stream::peer() const {
	return stream_.peer();
}

netkit::stream::socket_stream& netkit::uds::uds_stream::stream() {
	return stream_;
}

#endif