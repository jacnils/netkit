#include <netkit/stream/socket_stream.hpp>

void netkit::stream::socket_stream::connect() {
	if (!socket_)
		throw std::logic_error("socket closed");

	socket_->connect();
}

netkit::stream::stream_result netkit::stream::socket_stream::read(std::span<std::byte> buffer) {
	if (!socket_)
		throw std::logic_error("socket closed");

	auto result = socket_->recv(
		buffer.data(),
		buffer.size()
	);

	if (result == 0) {
		return {
			0,
			stream_status::eof
		};
	}

	if (result < 0) {
		return {
			0,
			stream_status::error
		};
	}

	return {
		static_cast<std::size_t>(result),
		stream_status::success
	};
}

netkit::stream::stream_result netkit::stream::socket_stream::write(std::span<const std::byte> buffer) {
	if (!socket_)
		throw std::logic_error("socket closed");

	auto result = socket_->send(
		buffer.data(),
		buffer.size()
	);

	if (result < 0) {
		return {
			0,
			stream_status::error
		};
	}

	return {
		static_cast<std::size_t>(result),
		stream_status::success
	};
}

void netkit::stream::socket_stream::close() noexcept {
	if (socket_) {
		socket_->close();
		socket_.reset();
	}
}

bool netkit::stream::socket_stream::is_open() const noexcept {
	if (socket_)
		return socket_->is_open();

	return false;
}

netkit::socket::addr netkit::stream::socket_stream::peer() const {
	if (!socket_)
		throw std::logic_error("socket closed");

	return socket_->get_peer();
}
