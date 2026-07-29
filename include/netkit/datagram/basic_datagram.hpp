#pragma once

#include <netkit/io/task.hpp>
#include <netkit/socket/addr.hpp>

#include <span>

namespace netkit::datagram {

class basic_datagram {
public:
	virtual ~basic_datagram() = default;

	virtual std::size_t send_to(std::span<const std::byte> buffer, const sock::addr& to) = 0;
	virtual std::pair<std::size_t, sock::addr> recv_from(std::span<std::byte> buffer) = 0;

	virtual void close() noexcept = 0;
};

}