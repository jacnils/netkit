#pragma once

#include <netkit/datagram/basic_async_datagram.hpp>

#include <memory>
#include <netkit/io/task.hpp>
#include <netkit/socket/native/basic_native_async_socket.hpp>

namespace netkit::udp {

class async_udp_datagram : public datagram::basic_async_datagram {
public:
	async_udp_datagram(io::io_context& ctx, const socket::addr& addr);

	void bind() const;

	io::task<std::size_t> send_to(std::span<const std::byte> buffer, const socket::addr& dest) override;

	io::task<std::pair<std::size_t, socket::addr>> recv_from(std::span<std::byte> buffer) override;

	void close() noexcept override;

	[[nodiscard]] bool is_open() const noexcept override;
private:
	socket::addr addr_;
	std::unique_ptr<socket::native::basic_native_async_socket> sock_;
};

}