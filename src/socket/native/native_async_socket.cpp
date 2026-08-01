/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol
 * abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file native_async_socket.cpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Implementation of the asynchronous socket class.
 */

#include <netkit/socket/native/native_async_sock.hpp>
#include <chrono>
#include <netkit/except.hpp>

#include <netkit/socket/native/peer_helper.hpp>
#include <netkit/platform/socket.hpp>

#if defined(NETKIT_UNIX) && !defined(NETKIT_DKP)
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>
#elif defined(NETKIT_WINDOWS)
#include <ws2tcpip.h>
#endif

void netkit::sock::native::native_async_sock::set_sock_opts(opt opts) {
	platform::set_sock_opts(this->sockfd, opts);
}

netkit::sock::native::native_async_sock::native_async_sock(netkit::io::io_context& ctx, const sock::addr& addr, sock::type t, opt opts)
	: addr_(addr), type_(t), context_(ctx) {

	if (!addr.is_file_path()) {
		if (addr.get_ip().empty()) {
			throw socket_error("IP address/file path is empty");
		}
	}

#ifdef NETKIT_DKP
	this->sockfd = platform::socket(addr.is_ipv6() ? AF_INET6 : AF_INET,
														  t == type::tcp ? SOCK_STREAM : SOCK_DGRAM, 0);
#else
    if (t != type::uds) {
        this->sockfd = platform::socket(addr.is_ipv6() ? AF_INET6 : AF_INET,
                                                          t == type::tcp ? SOCK_STREAM : SOCK_DGRAM, 0);
    } else {
        this->sockfd = platform::socket(AF_UNIX, SOCK_STREAM, 0);
    }
#endif

	if (!platform::valid_socket(sockfd)) {
		throw socket_error{"failed to create socket"};
	}

    this->native_async_sock::set_sock_opts(opts);
}

netkit::sock::native::native_async_sock::native_async_sock(netkit::io::io_context& ctx, fd_t existing_fd, const sock::addr& peer, sock::type t, opt opts)
    : addr_(peer), type_(t), sockfd(existing_fd), context_(ctx) {

	if (!platform::valid_socket(sockfd)) {
		throw socket_error{"invalid fd"};
	}

	this->native_async_sock::set_sock_opts(opts);
}

netkit::sock::native::native_async_sock::~native_async_sock() {
	this->native_async_sock::close();
}

netkit::sock::addr& netkit::sock::native::native_async_sock::get_addr() {
    return this->addr_;
}

const netkit::sock::addr& netkit::sock::native::native_async_sock::get_addr() const {
    return this->addr_;
}

netkit::io::task<void>
netkit::sock::native::native_async_sock::connect() {
	int ret = platform::connect(this->sockfd, addr_.get_sa(), addr_.get_sa_len());

	if (ret == 0) {
		co_return;
	}

	if (platform::last_socket_error() != platform::socket_err::would_block &&
		platform::last_socket_error() != platform::socket_err::in_progress) {
		throw netkit::socket_error{"failed to connect to server"};
	}

	co_await this->context_.wait_writable(this->sockfd);

	int error = 0;
	socklen_t len = sizeof(error);

	if (getsockopt(this->sockfd, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &len) < 0) {
		throw socket_error("getsockopt(SO_ERROR) failed");
	}

	if (error != 0) {
		throw socket_error("failed to connect to server: " + std::to_string(error));
	}

	co_return;
}

netkit::io::task<std::size_t> netkit::sock::native::native_async_sock::send(const void* buf, size_t len) {
	size_t total_sent = 0;
	const char* data = static_cast<const char*>(buf);

	while (total_sent < len) {
		auto sent = platform::send(
			this->sockfd,
			data + total_sent,
			len - total_sent,
			0
		);

		auto err = platform::last_socket_error();

		if (sent > 0) {
			total_sent += sent;
			continue;
		}

		if (sent == 0) {
			co_return total_sent;
		}

		if (err == platform::socket_err::would_block) {
			co_await context_.wait_writable(sockfd);
			continue;
		}

		if (err == platform::socket_err::interrupted) {
			continue;
		}

		throw socket_error("failed to send: " + platform::last_error_message());
	}

	co_return total_sent;
}

netkit::io::task<std::size_t>
netkit::sock::native::native_async_sock::recv(void* buf, size_t size) {
	for (;;) {
		auto n = platform::recv(this->sockfd, static_cast<char*>(buf), size, 0);

		if (n > 0) {
			co_return static_cast<size_t>(n);
		}

		if (n == 0) {
			co_return 0;
		}

		auto err = platform::last_socket_error();

		if (err == platform::socket_err::would_block) {
			co_await context_.wait_readable(sockfd);
			continue;
		}

		if (err == platform::socket_err::interrupted) {
			continue;
		}

		throw socket_error("recv failed: " + platform::last_error_message());
	}
}

void netkit::sock::native::native_async_sock::close() noexcept {
	if (platform::valid_socket(this->sockfd)) {
		platform::close_socket(this->sockfd);
		this->sockfd = platform::invalid_socket;
		this->bound = false;
	}
}

[[nodiscard]] netkit::sock::addr netkit::sock::native::native_async_sock::get_peer() const {
    return native::get_peer(this->sockfd);
}

netkit::sock::fd_t netkit::sock::native::native_async_sock::native_handle() const {
	return this->sockfd;
}

netkit::io::task<std::pair<std::size_t, netkit::sock::addr>>
netkit::sock::native::native_async_sock::recvfrom(void* buf, size_t size) {
	for (;;) {
		sockaddr_storage sa{};
		socklen_t sa_len = sizeof(sa);

		auto n = platform::recvfrom(
			this->sockfd,
			buf,
			size,
			0,
			reinterpret_cast<sockaddr*>(&sa),
			&sa_len
		);

		if (n >= 0) {
			addr from(reinterpret_cast<sockaddr*>(&sa), sa_len);

			co_return std::pair {
				static_cast<size_t>(n),
				std::move(from)
			};
		}

		auto err = platform::last_socket_error();

		if (err == platform::socket_err::would_block) {
			co_await context_.wait_readable(sockfd);
			continue;
		}

		if (err == platform::socket_err::interrupted) {
			continue;
		}

		throw socket_error(
			"recvfrom failed: " + platform::last_error_message()
		);
	}
}

netkit::io::task<std::size_t>
netkit::sock::native::native_async_sock::sendto(const void* buf, std::size_t len, const addr& dest) {
	while (true) {
		co_await context_.wait_writable(sockfd);

		auto ret = platform::sendto(
			sockfd,
			static_cast<const char*>(buf),
			static_cast<int>(len),
			0,
			dest.get_sa(),
			dest.get_sa_len()
		);

		if (!platform::valid_socket(ret)) {
			auto err = platform::last_socket_error();

			if (err == platform::socket_err::would_block)
				continue;

			throw socket_error("sendto failed: " + platform::last_error_message());
		}

		co_return static_cast<std::size_t>(ret);
	}
}

void netkit::sock::native::native_async_sock::bind() {
	if (platform::bind(sockfd, addr_.get_sa(), addr_.get_sa_len()) < 0) {
		throw socket_error("bind failed");
	}

	bound = true;
}

void netkit::sock::native::native_async_sock::bind(const addr& addr) {
	if (bound) {
		throw socket_error{"bind failed"};
	}

	if (platform::bind(sockfd, addr.get_sa(), addr.get_sa_len()) < 0) {
		throw socket_error("bind failed");
	}

	addr_ = addr;
	bound = true;
}

void netkit::sock::native::native_async_sock::unbind() noexcept {
	this->close();
}