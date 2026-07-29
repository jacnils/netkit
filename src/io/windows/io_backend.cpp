#include <netkit/except.hpp>

#include <algorithm>
#include <netkit/export.hpp>
#include <netkit/io/windows/io_backend.hpp>

#if defined(NETKIT_WINDOWS) && defined(NETKIT_WSAPOLL)

void netkit::io::io_backend::register_waiter(io_handle_t fd, io_event event, std::coroutine_handle<> h) {
	std::lock_guard lock(mutex_);

	waiters_.push_back({
		fd,
		event,
		h
	});
}

// TODO: move WSAStartup() to a single place, so that we don't call it multiple times.
netkit::io::io_backend::io_backend() {
	WSADATA data{};

	if (WSAStartup(MAKEWORD(2,2), &data) != 0)
		throw socket_error("WSAStartup failed");

	SOCKET listener = socket(
		AF_INET,
		SOCK_DGRAM,
		IPPROTO_UDP
	);

	if (listener == INVALID_SOCKET)
		throw socket_error("failed to create wake socket");


	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr.sin_port = 0;

	if (bind(
		listener,
		reinterpret_cast<sockaddr*>(&addr),
		sizeof(addr)
	) == SOCKET_ERROR) {
		closesocket(listener);
		throw socket_error("failed to bind wake socket");
	}


	int len = sizeof(addr);

	if (getsockname(
		listener,
		reinterpret_cast<sockaddr*>(&addr),
		&len
	) == SOCKET_ERROR) {
		closesocket(listener);
		throw socket_error("failed to get wake address");
	}

	wake_read_ = listener;

	wake_write_ = socket(
		AF_INET,
		SOCK_DGRAM,
		IPPROTO_UDP
	);

	if (wake_write_ == INVALID_SOCKET) {
		closesocket(wake_read_);
		throw socket_error("failed to create wake sender");
	}

	if (connect(
		wake_write_,
		reinterpret_cast<sockaddr*>(&addr),
		sizeof(addr)
	) == SOCKET_ERROR) {
		closesocket(wake_read_);
		closesocket(wake_write_);

		throw socket_error("failed to connect wake socket");
	}
}

netkit::io::io_backend::~io_backend() {
	io_backend::stop();

	if (wake_read_ != INVALID_SOCKET)
		closesocket(wake_read_);

	if (wake_write_ != INVALID_SOCKET)
		closesocket(wake_write_);

	WSACleanup();
}

void netkit::io::io_backend::wake() {
	char byte = 1;

	send(
		wake_write_,
		&byte,
		1,
		0
	);
}



void netkit::io::io_backend::poll(int timeout_ms) {
	std::vector<waiter> waiters;

	{
		std::lock_guard lock(mutex_);
		waiters = waiters_;
	}

	if (waiters.empty()) {
		Sleep(timeout_ms);
		return;
	}

	std::vector<WSAPOLLFD> fds;

	fds.reserve(waiters.size());

	for (auto& waiter : waiters) {
		WSAPOLLFD fd{};

		fd.fd = waiter.fd;

		if (waiter.event == io_event::read)
			fd.events = POLLRDNORM;

		else if (waiter.event == io_event::write)
			fd.events = POLLWRNORM;

		fds.push_back(fd);
	}

	int result = WSAPoll(
		fds.data(),
		static_cast<ULONG>(fds.size()),
		timeout_ms
	);

	if (result <= 0)
		return;

	std::vector<std::coroutine_handle<>> ready;

	{
		std::lock_guard lock(mutex_);

		for (std::size_t i = 0; i < fds.size(); i++) {
			auto revents = fds[i].revents;

			if (revents == 0)
				continue;

			auto it = std::ranges::find_if(waiters_,
				[&](const waiter& w) {
					return w.fd == waiters[i].fd &&
						   w.handle == waiters[i].handle;
				}
			);

			if (it != waiters_.end()) {
				ready.push_back(it->handle);
				waiters_.erase(it);
			}
		}
	}

	for (auto handle : ready) {
		if (handle)
			handle.resume();
	}
}

void netkit::io::io_backend::poll() {
	this->poll(-1);
}

void netkit::io::io_backend::run() {
	while (running_) {
		poll(-1);
	}
}

void netkit::io::io_backend::stop() {
	running_ = false;
	wake();
}

#endif