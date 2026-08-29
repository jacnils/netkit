#include <netkit/stream/async_socket_stream.hpp>
#include <netkit/io/task.hpp>
#include <netkit/socket/native/native_async_socket.hpp>

netkit::io::task<void>
netkit::stream::async_socket_stream::connect() const {
	co_await socket_->connect();
}

netkit::io::task<netkit::stream::stream_result>
netkit::stream::async_socket_stream::read(std::span<std::byte> buffer) {
	auto result = co_await socket_->recv(buffer.data(), buffer.size());

	if (result == 0) {
		co_return stream_result{ 0, stream_status::eof };
	}

	co_return stream_result{ result, stream_status::success };
}

netkit::io::task<netkit::stream::stream_result>
netkit::stream::async_socket_stream::write(std::span<const std::byte> buffer) {
	auto result = co_await socket_->send(buffer.data(), buffer.size());
	co_return stream_result{ result, result == 0 ? stream_status::eof : stream_status::success };
}

void netkit::stream::async_socket_stream::close() noexcept {
	if (socket_) {
		socket_->close();
		socket_.reset();
	}
}

bool netkit::stream::async_socket_stream::is_open() const noexcept {
	if (socket_)
		return socket_->is_open();

	return false;
}

netkit::socket::addr netkit::stream::async_socket_stream::peer() const {
	if (socket_)
		return socket_->get_peer();

	throw std::runtime_error{"peer(): socket_ not initialized"};
}
