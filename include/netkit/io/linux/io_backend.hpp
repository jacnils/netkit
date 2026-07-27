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
	void update_state(int fd, const io_handle_state& state) override;
	void register_waiter(int fd, io_event ev, std::coroutine_handle<> h) override;
	void run() override;
	void stop() override;
	void poll(int timeout_ms) override;
	void poll() override;
private:
	int epoll_fd_;
	bool running_ = true;
	int wake_fd_ = -1;

	std::unordered_map<int, io_handle_state> fd_map_;
	std::unordered_set<int> registered_fds_;
};

} // namespace netkit::io

#endif