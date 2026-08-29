#include <netkit/socket/native/native_sync_socket.hpp>
#include <netkit/udp/udp_datagram.hpp>

netkit::udp::udp_datagram::udp_datagram(const socket::addr& addr)
: addr_(addr), sock_(std::make_unique<socket::native::native_sync_socket>(addr, socket::type::udp, socket::opt::reuse_addr | socket::opt::blocking))
{}

void netkit::udp::udp_datagram::bind() const {
	sock_->bind(addr_);
}

std::size_t netkit::udp::udp_datagram::send_to(std::span<const std::byte> buffer, const socket::addr& dest) {
	return sock_->sendto(
		buffer.data(),
		buffer.size(),
		dest
	);
}

std::pair<std::size_t, netkit::socket::addr>
netkit::udp::udp_datagram::recv_from(std::span<std::byte> buffer) {
	if (sock_)
		return sock_->recvfrom(
			buffer.data(),
			buffer.size()
		);

	throw std::runtime_error{"recv_from() failed: sock_ == nullptr"};
}
void netkit::udp::udp_datagram::close() noexcept {
	if (sock_)
		sock_->close();
}

bool netkit::udp::udp_datagram::is_open() const noexcept {
	if (sock_)
		return sock_->is_open();

	return false;
}
