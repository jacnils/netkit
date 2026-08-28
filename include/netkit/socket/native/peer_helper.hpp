#pragma once

#include <netkit/socket/addr.hpp>

namespace netkit::socket::native {
	[[nodiscard]] netkit::socket::addr get_peer(fd_t fd);
}