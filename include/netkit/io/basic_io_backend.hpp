#pragma once

#include <coroutine>
#include <vector>

namespace netkit::io {

struct io_handle_state {
	std::vector<std::coroutine_handle<>> read_waiters;
	std::vector<std::coroutine_handle<>> write_waiters;
};

enum class io_event { read, write };

class basic_io_backend {
public:
	virtual ~basic_io_backend() = default;
	virtual void wake() = 0;
	virtual void update_state(int fd, const io_handle_state& state) = 0;
	virtual void register_waiter(int fd, io_event ev, std::coroutine_handle<> h) = 0;
	virtual void run() = 0;
	virtual void stop() = 0;
	virtual void poll(int timeout_ms) = 0;
	virtual void poll() = 0;
};

} // namespace netkit::io