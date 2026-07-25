#include <netkit/io/basic_io_context.hpp>

#ifdef NETKIT_LINUX
void netkit::io::basic_io_context::poll(int timeout_ms)
{
	epoll_event events[64];

	int n = epoll_wait(
		epoll_fd_,
		events,
		64,
		timeout_ms
	);

	for (int i = 0; i < n; ++i) {
		int fd = events[i].data.fd;

		if (fd == wake_fd_) {
			uint64_t value;

			read(wake_fd_, &value, sizeof(value));

			continue;
		}


		auto it = fd_map_.find(fd);

		if (it == fd_map_.end())
			continue;

		auto& state = it->second;

		if (events[i].events & EPOLLIN) {
			auto waiters = std::move(state.read_waiters);
			state.read_waiters.clear();

			for (auto h : waiters) {
				h.resume();
			}
		}

		if (events[i].events & EPOLLOUT) {
			auto waiters = std::move(state.write_waiters);
			state.write_waiters.clear();

			for (auto h : waiters) {
				h.resume();
			}
		}

		update_epoll(fd, state);
	}
}
#endif