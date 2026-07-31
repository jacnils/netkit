#pragma once

#include <netkit/datagram/basic_datagram.hpp>

#include <memory>
#include <netkit/socket/native/basic_native_sync_sock.hpp>

namespace netkit::udp {

class udp_datagram : public datagram::basic_datagram {
public:
	explicit udp_datagram(sock::addr addr);
	udp_datagram();

	void bind() const;

	std::size_t send_to(std::span<const std::byte> buffer, const sock::addr& dest) override;

	std::pair<std::size_t, sock::addr> recv_from(std::span<std::byte> buffer) override;

	void close() noexcept override;

private:
	sock::addr addr_;
	std::unique_ptr<sock::native::basic_native_sync_sock> sock_;
};

}