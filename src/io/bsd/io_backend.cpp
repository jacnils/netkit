#include <netkit/io/bsd/io_backend.hpp>

#if (defined(NETKIT_MACOS) || defined(NETKIT_BSD)) \
	&& defined(NETKIT_KQUEUE)

#include <coroutine>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <unordered_map>
#include <vector>
#include <cerrno>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/types.h>

#include <netkit/io/bsd/io_backend.hpp>

netkit::io::io_backend::io_backend() {
	kqueue_fd_ = kqueue();

	if (kqueue_fd_ == -1)
		throw std::runtime_error("kqueue failed");

	struct kevent ev{};

	EV_SET(
		&ev,
		wake_ident,
		EVFILT_USER,
		EV_ADD | EV_CLEAR,
		0,
		0,
		nullptr
	);

	if (kevent(kqueue_fd_, &ev, 1, nullptr, 0, nullptr) == -1) {
		close(kqueue_fd_);
		kqueue_fd_ = -1;

		throw std::runtime_error("failed to add kqueue wake event");
	}
}

netkit::io::io_backend::~io_backend() {
	if (kqueue_fd_ != -1)
		close(kqueue_fd_);
}

void netkit::io::io_backend::wake() {
	struct kevent ev{};

	EV_SET(
		&ev,
		wake_ident,
		EVFILT_USER,
		0,
		NOTE_TRIGGER,
		0,
		nullptr
	);

	kevent(kqueue_fd_, &ev, 1, nullptr, 0, nullptr);
}

void netkit::io::io_backend::update_state(
	io_handle_t fd,
	const io_handle_state& state
) {
	const bool wants_read = !state.read_waiters.empty();
	const bool wants_write = !state.write_waiters.empty();

	if (wants_read) {
		struct kevent ev{};

		EV_SET(
			&ev,
			static_cast<uintptr_t>(fd),
			EVFILT_READ,
			EV_ADD | EV_ENABLE,
			0,
			0,
			nullptr
		);

		if (kevent(kqueue_fd_, &ev, 1, nullptr, 0, nullptr) == -1)
			throw std::runtime_error(
				"failed to add kqueue read filter: " +
				std::string(std::strerror(errno))
			);
	} else {
		struct kevent ev{};

		EV_SET(
			&ev,
			static_cast<uintptr_t>(fd),
			EVFILT_READ,
			EV_DELETE,
			0,
			0,
			nullptr
		);

		if (kevent(kqueue_fd_, &ev, 1, nullptr, 0, nullptr) == -1 &&
		    errno != ENOENT) {
			throw std::runtime_error("failed to remove kqueue read filter: " + std::string(std::strerror(errno)));
		}
	}

	if (wants_write) {
		struct kevent ev{};

		EV_SET(
			&ev,
			static_cast<uintptr_t>(fd),
			EVFILT_WRITE,
			EV_ADD | EV_ENABLE,
			0,
			0,
			nullptr
		);

		if (kevent(kqueue_fd_, &ev, 1, nullptr, 0, nullptr) == -1)
			throw std::runtime_error(
				"failed to add kqueue write filter: " +
				std::string(std::strerror(errno))
			);
	} else {
		struct kevent ev{};

		EV_SET(
			&ev,
			static_cast<uintptr_t>(fd),
			EVFILT_WRITE,
			EV_DELETE,
			0,
			0,
			nullptr
		);

		if (kevent(kqueue_fd_, &ev, 1, nullptr, 0, nullptr) == -1 &&
		    errno != ENOENT) {
			throw std::runtime_error(
				"failed to remove kqueue write filter: " +
				std::string(std::strerror(errno))
			);
		}
	}

	if (wants_read || wants_write)
		registered_fds_.insert(fd);
	else
		registered_fds_.erase(fd);
}

void netkit::io::io_backend::register_waiter(
	io_handle_t fd,
	io_event ev,
	std::coroutine_handle<> h
) {
	auto& state = fd_map_[fd];

	if (ev == io_event::read)
		state.read_waiters.push_back(h);
	else
		state.write_waiters.push_back(h);

	update_state(fd, state);
}

void netkit::io::io_backend::run() {
	constexpr int MAX_EVENTS = 64;

	struct kevent events[MAX_EVENTS];

	while (running_) {
		int n = kevent(
			kqueue_fd_,
			nullptr,
			0,
			events,
			MAX_EVENTS,
			nullptr
		);

		if (n == -1) {
			if (errno == EINTR)
				continue;

			throw std::runtime_error("kevent failed");
		}

		for (int i = 0; i < n; ++i) {
			const auto& event = events[i];

			if (event.filter == EVFILT_USER &&
			    event.ident == wake_ident) {
				continue;
			}

			const io_handle_t fd =
				static_cast<io_handle_t>(event.ident);

			auto it = fd_map_.find(fd);

			if (it == fd_map_.end())
				continue;

			auto& state = it->second;

			const bool error =
				(event.flags & (EV_EOF | EV_ERROR)) != 0;

			if (event.filter == EVFILT_READ || error) {
				auto waiters =
					std::move(state.read_waiters);

				state.read_waiters.clear();

				for (auto h : waiters)
					h.resume();
			}

			if (event.filter == EVFILT_WRITE || error) {
				auto waiters =
					std::move(state.write_waiters);

				state.write_waiters.clear();

				for (auto h : waiters)
					h.resume();
			}

			update_state(fd, state);
		}
	}
}

void netkit::io::io_backend::stop() {
	running_ = false;

	wake();
}

void netkit::io::io_backend::poll(int timeout_ms) {
	struct kevent events[64];

	timespec timeout{};

	if (timeout_ms >= 0) {
		timeout.tv_sec = timeout_ms / 1000;
		timeout.tv_nsec = (timeout_ms % 1000) * 1000000;
	}

	int n = kevent(
		kqueue_fd_,
		nullptr,
		0,
		events,
		64,
		timeout_ms < 0 ? nullptr : &timeout
	);

	if (n == -1) {
		if (errno == EINTR)
			return;

		throw std::runtime_error("kevent failed");
	}

	for (int i = 0; i < n; ++i) {
		const auto& event = events[i];

		if (event.filter == EVFILT_USER &&
		    event.ident == wake_ident) {
			continue;
		}

		const auto fd =
			static_cast<io_handle_t>(event.ident);

		auto it = fd_map_.find(fd);

		if (it == fd_map_.end())
			continue;

		auto& state = it->second;

		const bool error =
			(event.flags & (EV_EOF | EV_ERROR)) != 0;

		if (event.filter == EVFILT_READ || error) {
			auto waiters =
				std::move(state.read_waiters);

			state.read_waiters.clear();

			for (auto h : waiters)
				h.resume();
		}

		if (event.filter == EVFILT_WRITE || error) {
			auto waiters =
				std::move(state.write_waiters);

			state.write_waiters.clear();

			for (auto h : waiters)
				h.resume();
		}

		update_state(fd, state);
	}
}

void netkit::io::io_backend::poll() {
	poll(-1);
}

#endif