#pragma once

#include <netkit/definitions.hpp>

#ifdef NETKIT_LINUX

#include <coroutine>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <sys/epoll.h>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <sys/eventfd.h>

namespace netkit::io {

struct fd_state {
	std::vector<std::coroutine_handle<>> read_waiters;
	std::vector<std::coroutine_handle<>> write_waiters;
};

enum class io_event { read, write };

class basic_io_context {
public:
	basic_io_context() {
		epoll_fd_ = epoll_create1(0);

		if (epoll_fd_ == -1)
			throw std::runtime_error("epoll_create1 failed");

		wake_fd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);

		if (wake_fd_ == -1)
			throw std::runtime_error("eventfd failed");


		epoll_event ev{};
		ev.events = EPOLLIN;
		ev.data.fd = wake_fd_;

		if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, wake_fd_, &ev) == -1) {
			throw std::runtime_error(
				"failed to add wake fd"
			);
		}
	}

	~basic_io_context() {
		if (wake_fd_ != -1)
			close(wake_fd_);

		if (epoll_fd_ != -1)
			close(epoll_fd_);
	}

	void wake() {
		uint64_t value = 1;

		write(wake_fd_, &value, sizeof(value));
	}

	void update_epoll(int fd, const fd_state& state) {
		epoll_event ev{};
		ev.data.fd = fd;
		ev.events = 0;

		if (!state.read_waiters.empty())
			ev.events |= EPOLLIN;

		if (!state.write_waiters.empty())
			ev.events |= EPOLLOUT;

		if (ev.events == 0) {
			if (registered_fds_.contains(fd)) {
				epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
				registered_fds_.erase(fd);
			}
			return;
		}

		int ret;

		if (!registered_fds_.contains(fd)) {
			ret = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev);

			if (ret == -1)
				throw std::runtime_error(std::strerror(errno));

			registered_fds_.insert(fd);
		}
		else {
			ret = epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev);

			if (ret == -1)
				throw std::runtime_error(std::strerror(errno));
		}
	}

	void register_waiter(int fd, io_event ev, std::coroutine_handle<> h) {
		auto& state = fd_map_[fd];

		if (ev == io_event::read)
			state.read_waiters.push_back(h);
		else
			state.write_waiters.push_back(h);

		update_epoll(fd, state);
	}

	void run() {
		constexpr int MAX_EVENTS = 64;
		epoll_event events[MAX_EVENTS];

		while (running_) {
			int n = epoll_wait(epoll_fd_, events, MAX_EVENTS, -1);

			if (n == -1) {
				if (errno == EINTR) continue;
				throw std::runtime_error("epoll_wait failed");
			}

			for (int i = 0; i < n; ++i) {
				int fd = events[i].data.fd;

				auto it = fd_map_.find(fd);
				if (it == fd_map_.end()) continue;

				auto& state = it->second;

				if (events[i].events & EPOLLIN) {
					auto waiters = std::move(state.read_waiters);
					state.read_waiters.clear();

					for (auto h : waiters)
						h.resume();
				}

				if (events[i].events & EPOLLOUT) {
					auto waiters = std::move(state.write_waiters);
					state.write_waiters.clear();

					for (auto h : waiters)
						h.resume();
				}

				if (events[i].events & (EPOLLERR | EPOLLHUP)) {
					auto read_waiters = std::move(state.read_waiters);
					auto write_waiters = std::move(state.write_waiters);

					state.read_waiters.clear();
					state.write_waiters.clear();

					for (auto h : read_waiters) h.resume();
					for (auto h : write_waiters) h.resume();

					update_epoll(fd, state);
					continue;
				}

				update_epoll(fd, state);
			}
		}
	}

	void stop() {
		running_ = false;
	}

	void poll(int timeout_ms = -1);

private:
	int epoll_fd_;
	bool running_ = true;
	int wake_fd_ = -1;

	std::unordered_map<int, fd_state> fd_map_;
	std::unordered_set<int> registered_fds_;
};

} // namespace netkit::io

#endif