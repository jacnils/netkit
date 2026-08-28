#pragma once

#include <netkit/platform/socket.hpp>
#include <netkit/datagram/basic_datagram.hpp>
#include <netkit/socket/native/basic_native_sync_sock.hpp>

namespace netkit::datagram {

class socket_datagram : public basic_datagram {
public:
	explicit socket_datagram(std::unique_ptr<socket::native::basic_native_sync_socket> socket)
		: sock_(std::move(socket)) {}

	std::size_t
	send_to(std::span<const std::byte> buffer, const socket::addr& to) override {
		return sock_->sendto(buffer.data(), buffer.size(), to);
	}

	std::pair<std::size_t, socket::addr>
	recv_from(std::span<std::byte> buffer) override {
		return sock_->recvfrom(buffer.data(), buffer.size());
	}

	void close() noexcept override {
		sock_->close();
	}

	platform::socket_t native_handle() const noexcept {
		return sock_->native_handle();
	}

private:
	std::unique_ptr<socket::native::basic_native_sync_socket> sock_;
};

}