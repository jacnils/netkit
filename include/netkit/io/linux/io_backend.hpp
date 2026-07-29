#pragma once

#include <netkit/definitions.hpp>

#if defined(NETKIT_LINUX) && defined(NETKIT_EPOLL)

#include <netkit/io/basic_io_backend.hpp>

#include <coroutine>
#include <unordered_map>
#include <unordered_set>

namespace netkit::io {

class io_backend : public basic_io_backend {
public:
	io_backend();
	~io_backend() override;

	void wake() override;
	void update_state(io_handle_t fd, const io_handle_state& state) override;
	void register_waiter(io_handle_t fd, io_event ev, std::coroutine_handle<> h) override;
	void run() override;
	void stop() override;
	void poll(int timeout_ms) override;
	void poll() override;
private:
	io_handle_t epoll_fd_;
	bool running_ = true;
	io_handle_t wake_fd_ = -1;

	std::unordered_map<io_handle_t, io_handle_state> fd_map_;
	std::unordered_set<io_handle_t> registered_fds_;
};

} // namespace netkit::io

#endif