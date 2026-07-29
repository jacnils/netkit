#pragma once

#include <netkit/definitions.hpp>

#include <coroutine>
#include <vector>

#ifdef NETKIT_WINDOWS
#include <winsock2.h>
#endif

namespace netkit::io {

struct io_handle_state {
	std::vector<std::coroutine_handle<>> read_waiters;
	std::vector<std::coroutine_handle<>> write_waiters;
};

enum class io_event { read, write };

#ifdef NETKIT_WINDOWS
typedef SOCKET io_handle_t;
#else
typedef int io_handle_t;
#endif

class basic_io_backend {
public:
	virtual ~basic_io_backend() = default;
	virtual void wake() = 0;
	virtual void update_state(io_handle_t fd, const io_handle_state& state) = 0;
	virtual void register_waiter(io_handle_t fd, io_event ev, std::coroutine_handle<> h) = 0;
	virtual void run() = 0;
	virtual void stop() = 0;
	virtual void poll(int timeout_ms) = 0;
	virtual void poll() = 0;
};

} // namespace netkit::io