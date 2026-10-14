#pragma once

#include "netkit/udp/async_udp_datagram.hpp"

#include <netkit/export.hpp>
#include <netkit/io/task.hpp>
#include <netkit/socket/addr.hpp>
#include <span>

namespace netkit::datagram {

class NETKIT_API basic_async_datagram {
public:
	virtual ~basic_async_datagram() = default;

	virtual io::task<platform::socket_result> send_to(std::span<const std::byte> buffer,
													  const socket::addr&		 to) = 0;

	virtual io::task<std::pair<platform::socket_result, socket::addr>>
	recv_from(std::span<std::byte> buffer) = 0;

	virtual void close() noexcept = 0;
	[[nodiscard]] virtual bool is_open() const noexcept = 0;
};

}