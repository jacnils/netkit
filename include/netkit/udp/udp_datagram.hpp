#pragma once

#include <netkit/datagram/basic_datagram.hpp>

#include <memory>
#include <netkit/socket/native/basic_native_sync_socket.hpp>

namespace netkit::udp {

class udp_datagram : public datagram::basic_datagram {
public:
	explicit udp_datagram(const socket::addr& addr);

	void bind() const;

	std::size_t send_to(std::span<const std::byte> buffer, const socket::addr& dest) override;

	std::pair<std::size_t, socket::addr> recv_from(std::span<std::byte> buffer) override;

	void close() noexcept override;

	[[nodiscard]] bool is_open() const noexcept override;
private:
	socket::addr addr_;
	std::unique_ptr<socket::native::basic_native_sync_socket> sock_;
};

}