#pragma once

#include <netkit/io/task.hpp>
#include <netkit/socket/addr.hpp>

#include <span>

namespace netkit::datagram {

class basic_async_datagram {
public:
	virtual ~basic_async_datagram() = default;

	virtual io::task<std::size_t>
	send_to(std::span<const std::byte> buffer, const sock::addr& to) = 0;

	virtual io::task<std::pair<std::size_t, sock::addr>>
	recv_from(std::span<std::byte> buffer) = 0;

	virtual void close() noexcept = 0;
};

}