#include <netkit/socket/native/native_async_sock.hpp>
#include <netkit/udp/async_udp_datagram.hpp>

netkit::udp::async_udp_datagram::async_udp_datagram(netkit::io::io_context& ctx, sock::addr addr)
: addr_(addr), sock_(std::make_unique<sock::native::native_async_sock>(ctx, addr, sock::type::udp))
{}

void netkit::udp::async_udp_datagram::bind() const {
	sock_->bind(addr_);
}

netkit::io::task<std::size_t> netkit::udp::async_udp_datagram::send_to(std::span<const std::byte> buffer, const sock::addr& dest) {
	return sock_->sendto(
		buffer.data(),
		buffer.size(),
		dest
	);
}

netkit::io::task<std::pair<std::size_t, netkit::sock::addr>>
netkit::udp::async_udp_datagram::recv_from(std::span<std::byte> buffer) {
	return sock_->recvfrom(
		buffer.data(),
		buffer.size()
	);
}

void netkit::udp::async_udp_datagram::close() noexcept {
	sock_->close();
}