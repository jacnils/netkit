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
#include <cstring>
#include <unistd.h>
#include <netkit/except.hpp>

#include <netkit/socket/native/peer_helper.hpp>
#include <netkit/platform/socket.hpp>

#ifdef NETKIT_UNIX
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#elif defined(NETKIT_WINDOWS)
#include <ws2tcpip.h>
#include <afunix.h>
#endif

const sockaddr* netkit::sock::native::native_async_sock::get_sa() const {
    return reinterpret_cast<const sockaddr*>(&sa_storage);
}

socklen_t netkit::sock::native::native_async_sock::get_sa_len() const {
    if (addr_.is_ipv4()) return sizeof(sockaddr_in);
    if (addr_.is_ipv6()) return sizeof(sockaddr_in6);
#ifndef NETKIT_DKP
    if (addr_.is_file_path()) {
        const auto& path = addr_.get_path();
        return static_cast<socklen_t>(offsetof(sockaddr_un, sun_path) + path.string().size() + 1);
    }
#endif

    throw netkit::socket_error("invalid address type");
}

void netkit::sock::native::native_async_sock::prep_sa() {
	memset(&sa_storage, 0, sizeof(sa_storage));

	if (addr_.is_ipv4()) {
		auto* sa4 = reinterpret_cast<sockaddr_in*>(&sa_storage);
		sa4->sin_family = AF_INET;
		sa4->sin_port = htons(addr_.get_port());
		if (inet_pton(AF_INET, addr_.get_ip().c_str(), &sa4->sin_addr) <= 0) {
			throw parsing_error("invalid IPv4 address");
		}
	} else if (addr_.is_ipv6()) {
		auto* sa6 = reinterpret_cast<sockaddr_in6*>(&sa_storage);
		sa6->sin6_family = AF_INET6;
		sa6->sin6_port = htons(addr_.get_port());

		std::string ip = addr_.get_ip();
		unsigned long scope = 0;

		auto pos = ip.find('%');
		if (pos != std::string::npos) {
			scope = std::stoul(ip.substr(pos + 1));
			ip = ip.substr(0, pos);   // strip %scope before inet_pton
		}

		if (inet_pton(AF_INET6, ip.c_str(), &sa6->sin6_addr) <= 0) {
			throw parsing_error("invalid IPv6 address");
		}

		if (scope != 0) {
			sa6->sin6_scope_id = scope;
		}
	} else if (addr_.is_file_path()) {
		auto* sa_un = reinterpret_cast<sockaddr_un*>(&sa_storage);
		sa_un->sun_family = AF_UNIX;
		const auto& path = addr_.get_path().string();
		if (path.size() >= sizeof(sa_un->sun_path)) {
			throw socket_error("UNIX socket path too long");
		}
		std::memcpy(sa_un->sun_path, path.c_str(), path.size() + 1);
	} else {
		throw ip_error("invalid address type");
	}
}

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

    if (t != type::uds) {
        this->sockfd = platform::socket(addr.is_ipv6() ? AF_INET6 : AF_INET,
                                                          t == type::tcp ? SOCK_STREAM : SOCK_DGRAM, 0);
    } else {
        this->sockfd = platform::socket(AF_UNIX, SOCK_STREAM, 0);
    }

	if (!platform::valid_socket(sockfd)) {
		throw socket_error{"failed to create socket"};
	}

    this->native_async_sock::set_sock_opts(opts);
    this->prep_sa();
}

netkit::sock::native::native_async_sock::native_async_sock(netkit::io::io_context& ctx, fd_t existing_fd, const sock::addr& peer, sock::type t, opt opts)
    : addr_(peer), type_(t), sockfd(existing_fd), context_(ctx) {

	if (!platform::valid_socket(sockfd)) {
		throw socket_error{"invalid fd"};
	}

	this->native_async_sock::set_sock_opts(opts);
    this->prep_sa();
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
	int ret = platform::connect(
		this->sockfd,
		this->get_sa(),
		this->get_sa_len()
	);

	if (ret == 0) {
		co_return;
	}

	if (platform::last_socket_error() != platform::socket_err::would_block &&
		platform::last_socket_error() != platform::socket_err::in_progress) {
		throw netkit::socket_error{"failed to connect to server"};
	}

	co_await this->context_.wait_writable(this->sockfd);

	int error = 0;
	int len = sizeof(error);

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
	}
}

[[nodiscard]] netkit::sock::addr netkit::sock::native::native_async_sock::get_peer() const {
    return native::get_peer(this->sockfd);
}

netkit::sock::fd_t netkit::sock::native::native_async_sock::native_handle() const {
	return this->sockfd;
}