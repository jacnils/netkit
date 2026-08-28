/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol
 * abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file native_sync_socket.cpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Implementation of the synchronous socket class.
 */
#include <netkit/platform/socket.hpp>

#include <iostream>
#include <netkit/except.hpp>
#include <netkit/socket/native/native_sync_socket.hpp>
#include <netkit/socket/native/peer_helper.hpp>

#ifdef NETKIT_WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#elif NETKIT_UNIX
#include <sys/socket.h>
#ifndef NETKIT_DKP
#include <sys/un.h>
#else
#include <network.h>
#include <mutex>
#endif
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#include <chrono>
#include <unordered_map>

void netkit::socket::native::native_sync_socket::connect() {
	if (platform::connect(sockfd, addr_.get_sa(), addr_.get_sa_len()) < 0) {
		throw socket_error("connect failed: " + platform::last_error_message());
	}
}

void netkit::socket::native::native_sync_socket::set_sock_opts(opt opts) {
	platform::set_sock_opts(this->sockfd, opts);
}

netkit::socket::native::native_sync_socket::native_sync_socket(const socket::addr& addr, socket::type t, opt opts) : addr_(addr), type_(t) {
	if (!addr.is_file_path()) {
		if (addr.get_ip().empty()) {
			throw socket_error("IP address/file path is empty");
		}
	}

#ifndef NETKIT_DKP
	if (this->type_ == type::uds) {
		sockfd = platform::socket(AF_UNIX, SOCK_STREAM, 0);
	} else {
		sockfd = platform::socket(addr_.is_ipv6() ? AF_INET6 : AF_INET, t == type::tcp ? SOCK_STREAM : SOCK_DGRAM, 0);
	}
#else
	sockfd = platform::socket(addr_.is_ipv6() ? AF_INET6 : AF_INET, t == type::tcp ? SOCK_STREAM : SOCK_DGRAM, 0);
#endif

	if (!platform::valid_socket(sockfd))
		throw socket_error{"failed to create socket"};

	this->native_sync_socket::set_sock_opts(opts);
}

netkit::socket::native::native_sync_socket::native_sync_socket(fd_t existing_fd, const socket::addr& peer, socket::type t, opt opts)
	: addr_(peer), type_(t), sockfd(existing_fd) {

	if (!platform::valid_socket(sockfd))
		throw socket_error{"invalid fd"};

	this->native_sync_socket::set_sock_opts(opts);
}

netkit::socket::native::native_sync_socket::~native_sync_socket() {
	this->native_sync_socket::close();
}

netkit::socket::addr& netkit::socket::native::native_sync_socket::get_addr() {
    return this->addr_;
}

const netkit::socket::addr& netkit::socket::native::native_sync_socket::get_addr() const {
    return this->addr_;
}

std::size_t netkit::socket::native::native_sync_socket::send(const void* buf, size_t len) {
	return platform::send(this->sockfd, static_cast<const char*>(buf), len, 0);
}

std::size_t netkit::socket::native::native_sync_socket::recv(void* buf, std::size_t len) {
	return platform::recv(this->sockfd, static_cast<char*>(buf), len, 0);
}

std::pair<std::size_t, netkit::socket::addr>
netkit::socket::native::native_sync_socket::recvfrom(void* buf, size_t size) {
	sockaddr_storage sa{};
	socklen_t sa_len = sizeof(sa);

	auto ret = platform::recvfrom(this->sockfd, buf, size, 0, reinterpret_cast<sockaddr*>(&sa), &sa_len);

	if (ret < 0) {
		throw socket_error("recvfrom failed: " + platform::last_error_message());
	}

	addr from(reinterpret_cast<sockaddr*>(&sa), sa_len);

	return {
		static_cast<std::size_t>(ret),
		std::move(from)
	};
}


std::size_t
netkit::socket::native::native_sync_socket::sendto(const void* buf, std::size_t len, const addr& dest) {
	auto ret = platform::sendto(this->sockfd, static_cast<const char*>(buf), static_cast<int>(len), 0, dest.get_sa(), dest.get_sa_len());

	if (ret < 0) {
		throw socket_error("sendto failed: " + platform::last_error_message());
	}

	return static_cast<std::size_t>(ret);
}

void netkit::socket::native::native_sync_socket::close() noexcept {
	if (!platform::valid_socket(this->sockfd)) {
		return;
	}

	this->bound = false;

	platform::close_socket(this->sockfd);
}

[[nodiscard]] netkit::socket::addr netkit::socket::native::native_sync_socket::get_peer() const {
	return native::get_peer(this->sockfd);
}

netkit::socket::fd_t netkit::socket::native::native_sync_socket::native_handle() const {
	return this->sockfd;
}

void netkit::socket::native::native_sync_socket::bind() {
	if (platform::bind(sockfd, addr_.get_sa(), addr_.get_sa_len()) < 0) {
		throw socket_error("bind failed");
	}

	bound = true;
}

void netkit::socket::native::native_sync_socket::bind(const addr& addr) {
	if (bound) {
		throw socket_error{"bind failed"};
	}

	if (platform::bind(sockfd, addr.get_sa(), addr.get_sa_len()) < 0) {
		throw socket_error("bind failed");
	}

	addr_ = addr;
	bound = true;
}

void netkit::socket::native::native_sync_socket::unbind() noexcept {
	this->close();
}