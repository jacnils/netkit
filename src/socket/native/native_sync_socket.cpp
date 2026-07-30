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
#include <netkit/socket/native/native_sync_sock.hpp>
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

void netkit::sock::native::native_sync_sock::connect() {
	if (platform::connect(sockfd, addr_.get_sa(), addr_.get_sa_len()) < 0) {
		throw socket_error("connect failed: " + platform::last_error_message());
	}
}

void netkit::sock::native::native_sync_sock::set_sock_opts(opt opts) {
	platform::set_sock_opts(this->sockfd, opts);
}

netkit::sock::native::native_sync_sock::native_sync_sock(const sock::addr& addr, sock::type t, opt opts) : addr_(addr), type_(t) {
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

	if (!platform::valid_socket(sockfd))
		throw socket_error{"failed to create socket"};

	this->native_sync_sock::set_sock_opts(opts);
}

netkit::sock::native::native_sync_sock::native_sync_sock(fd_t existing_fd, const sock::addr& peer, sock::type t, opt opts)
	: addr_(peer), type_(t), sockfd(existing_fd) {

	if (!platform::valid_socket(sockfd))
		throw socket_error{"invalid fd"};

	this->native_sync_sock::set_sock_opts(opts);
}

netkit::sock::native::native_sync_sock::~native_sync_sock() {
	this->native_sync_sock::close();
}

netkit::sock::addr& netkit::sock::native::native_sync_sock::get_addr() {
    return this->addr_;
}

const netkit::sock::addr& netkit::sock::native::native_sync_sock::get_addr() const {
    return this->addr_;
}

std::size_t netkit::sock::native::native_sync_sock::send(const void* buf, size_t len) {
	return platform::send(this->sockfd, static_cast<const char*>(buf), len, 0);
}

std::size_t netkit::sock::native::native_sync_sock::recv(void* buf, std::size_t len) {
	return platform::recv(this->sockfd, static_cast<char*>(buf), len, 0);
}

std::pair<std::size_t, netkit::sock::addr>
netkit::sock::native::native_sync_sock::recvfrom(void* buf, size_t size) {
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
netkit::sock::native::native_sync_sock::sendto(const void* buf, std::size_t len, const addr& dest) {
	auto ret = platform::sendto(this->sockfd, static_cast<const char*>(buf), static_cast<int>(len), 0, dest.get_sa(), dest.get_sa_len());

	if (ret < 0) {
		throw socket_error("sendto failed: " + platform::last_error_message());
	}

	return static_cast<std::size_t>(ret);
}

void netkit::sock::native::native_sync_sock::close() noexcept {
	if (!platform::valid_socket(this->sockfd)) {
		return;
	}

	this->bound = false;

	platform::close_socket(this->sockfd);
}

[[nodiscard]] netkit::sock::addr netkit::sock::native::native_sync_sock::get_peer() const {
	return native::get_peer(this->sockfd);
}

netkit::sock::fd_t netkit::sock::native::native_sync_sock::native_handle() const {
	return this->sockfd;
}

void netkit::sock::native::native_sync_sock::bind() {
	if (platform::bind(sockfd, addr_.get_sa(), addr_.get_sa_len()) < 0) {
		throw socket_error("bind failed");
	}

	bound = true;
}

void netkit::sock::native::native_sync_sock::bind(const addr& addr) {
	if (bound) {
		throw socket_error{"bind failed"};
	}

	if (platform::bind(sockfd, addr.get_sa(), addr.get_sa_len()) < 0) {
		throw socket_error("bind failed");
	}

	addr_ = addr;
	bound = true;
}

void netkit::sock::native::native_sync_sock::unbind() noexcept {
	this->close();
}