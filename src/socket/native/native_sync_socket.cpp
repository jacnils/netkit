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

#include <cstring>
#include <iostream>
#include <netkit/except.hpp>
#include <netkit/socket/native/native_sync_sock.hpp>
#include <netkit/socket/native/peer_helper.hpp>

#ifdef NETKIT_WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <afunix.h>
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

const sockaddr* netkit::sock::native::native_sync_sock::get_sa() const {
    return reinterpret_cast<const sockaddr*>(&sa_storage);
}

socklen_t netkit::sock::native::native_sync_sock::get_sa_len() const {
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

void netkit::sock::native::native_sync_sock::prep_sa() {
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
#ifndef NETKIT_DKP
	} else if (addr_.is_file_path()) {
		auto* sa_un = reinterpret_cast<sockaddr_un*>(&sa_storage);
		sa_un->sun_family = AF_UNIX;
		const auto& path = addr_.get_path().string();
		if (path.size() >= sizeof(sa_un->sun_path)) {
			throw socket_error("UNIX socket path too long");
		}
		std::memcpy(sa_un->sun_path, path.c_str(), path.size() + 1);
#endif
	} else {
		throw ip_error("invalid address type");
	}
}

void netkit::sock::native::native_sync_sock::connect() {
	if (platform::connect(sockfd, get_sa(), get_sa_len()) < 0) {
		throw socket_error("connect failed: " + platform::last_error_message());
	}
}

void netkit::sock::native::native_sync_sock::set_sock_opts(opt opts) {
	platform::set_sock_opts(this->sockfd, opts);
}

netkit::sock::native::native_sync_sock::native_sync_sock(const sock::addr& addr, sock::type t, opt opts) : addr_(addr), type_(t) {
	this->sockfd = -1;

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
	this->prep_sa();
}

netkit::sock::native::native_sync_sock::native_sync_sock(fd_t existing_fd, const sock::addr& peer, sock::type t, opt opts)
	: addr_(peer), type_(t), sockfd(existing_fd) {

	if (!platform::valid_socket(sockfd))
		throw socket_error{"invalid fd"};

	this->native_sync_sock::set_sock_opts(opts);
	this->prep_sa();
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

void netkit::sock::native::native_sync_sock::close() noexcept {
	if (!platform::valid_socket(this->sockfd)) {
		return;
	}

	platform::close_socket(this->sockfd);
}

[[nodiscard]] netkit::sock::addr netkit::sock::native::native_sync_sock::get_peer() const {
	return native::get_peer(this->sockfd);
}

netkit::sock::fd_t netkit::sock::native::native_sync_sock::native_handle() const {
	return this->sockfd;
}