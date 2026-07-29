#pragma once

#include <netkit/datagram/basic_async_datagram.hpp>

#include <memory>
#include <netkit/io/task.hpp>
#include <netkit/socket/native/basic_native_async_sock.hpp>

namespace netkit::udp {

class async_udp_datagram : public datagram::basic_async_datagram {
public:
	async_udp_datagram(
		io::io_context& ctx,
		sock::addr addr
	);

	void bind() const;

	io::task<std::size_t>
	send_to(
		std::span<const std::byte> buffer,
		const sock::addr& dest
	);

	io::task<std::pair<std::size_t, sock::addr>>
	recv_from(
		std::span<std::byte> buffer
	);

	void close() noexcept;

private:
	sock::addr addr_;
	std::unique_ptr<sock::native::basic_native_async_sock> sock_;
};

}