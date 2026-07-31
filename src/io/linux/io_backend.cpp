#include <netkit/io/linux/io_backend.hpp>

#if defined(NETKIT_LINUX) && defined(NETKIT_EPOLL)

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

netkit::io::io_backend::io_backend() {
	epoll_fd_ = epoll_create1(0);

	if (epoll_fd_ == -1)
		throw std::runtime_error("epoll_create1 failed");

	wake_fd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);

	if (wake_fd_ == -1)
		throw std::runtime_error("eventfd failed");

	epoll_event ev{};
	ev.events  = EPOLLIN;
	ev.data.fd = wake_fd_;

	if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, wake_fd_, &ev) == -1) {
		throw std::runtime_error("failed to add wake fd");
	}
}

netkit::io::io_backend::~io_backend() {
	if (wake_fd_ != -1)
		close(wake_fd_);

	if (epoll_fd_ != -1)
		close(epoll_fd_);
}

void netkit::io::io_backend::wake() {
	uint64_t value = 1;

	write(wake_fd_, &value, sizeof(value));
}

void netkit::io::io_backend::update_state(int fd, const io_handle_state& state) {
	epoll_event ev{};
	ev.data.fd = fd;
	ev.events  = 0;

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
	} else {
		ret = epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev);

		if (ret == -1)
			throw std::runtime_error(std::strerror(errno));
	}
}

void netkit::io::io_backend::register_waiter(int fd, io_event ev, std::coroutine_handle<> h) {
	auto& state = fd_map_[fd];

	if (ev == io_event::read)
		state.read_waiters.push_back(h);
	else
		state.write_waiters.push_back(h);

	update_state(fd, state);
}

void netkit::io::io_backend::run() {
	constexpr int MAX_EVENTS = 64;
	epoll_event	  events[MAX_EVENTS];

	while (running_) {
		int n = epoll_wait(epoll_fd_, events, MAX_EVENTS, -1);

		if (n == -1) {
			if (errno == EINTR)
				continue;
			throw std::runtime_error("epoll_wait failed");
		}

		for (int i = 0; i < n; ++i) {
			int fd = events[i].data.fd;

			auto it = fd_map_.find(fd);
			if (it == fd_map_.end())
				continue;

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
				auto read_waiters  = std::move(state.read_waiters);
				auto write_waiters = std::move(state.write_waiters);

				state.read_waiters.clear();
				state.write_waiters.clear();

				for (auto h : read_waiters)
					h.resume();
				for (auto h : write_waiters)
					h.resume();

				update_state(fd, state);
				continue;
			}

			update_state(fd, state);
		}
	}
}

void netkit::io::io_backend::stop() {
	running_ = false;
}

void netkit::io::io_backend::poll(int timeout_ms) {
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

		update_state(fd, state);
	}
}

void netkit::io::io_backend::poll() {
	this->poll(-1);
}

#endif