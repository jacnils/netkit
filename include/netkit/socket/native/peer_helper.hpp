#pragma once

#include <netkit/socket/addr.hpp>

namespace netkit::sock::native {
	[[nodiscard]] netkit::sock::addr get_peer(fd_t fd);
}