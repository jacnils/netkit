#pragma once

#include <netkit/export.hpp>
#include <netkit/socket/addr.hpp>

namespace netkit::socket::native {
	[[nodiscard]] NETKIT_API netkit::socket::addr get_peer(fd_t fd);
}