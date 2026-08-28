#pragma once

#include <netkit/platform/socket.hpp>
#include <netkit/datagram/basic_async_datagram.hpp>
#include <netkit/socket/native/basic_native_async_socket.hpp>

namespace netkit::datagram {

class async_socket_datagram : public basic_async_datagram {
public:
	explicit async_socket_datagram(std::unique_ptr<socket::native::basic_native_async_socket> socket)
		: sock_(std::move(socket)) {}

	io::task<std::size_t>
	send_to(std::span<const std::byte> buffer, const socket::addr& to) override {
		co_return co_await sock_->sendto(buffer.data(), buffer.size(), to);
	}

	io::task<std::pair<std::size_t, socket::addr>>
	recv_from(std::span<std::byte> buffer) override {
		co_return co_await sock_->recvfrom(buffer.data(), buffer.size());
	}

	void close() noexcept override {
		sock_->close();
	}

	platform::socket_t native_handle() const noexcept {
		return sock_->native_handle();
	}

private:
	std::unique_ptr<socket::native::basic_native_async_socket> sock_;
};

}