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
	if (::connect(sockfd, get_sa(), get_sa_len()) < 0) {
		throw socket_error("connect failed: " + std::string(strerror(errno)));
	}
}

void netkit::sock::native::native_sync_sock::set_sock_opts(opt opts) {
	netkit::platform::set_sock_opts(this->sockfd, opts);
}

#ifdef NETKIT_UNIX
netkit::sock::native::native_sync_sock::native_sync_sock(const sock::addr& addr, sock::type t, opt opts) : addr_(addr), type_(t) {
    this->sockfd = -1;

	if (!addr.is_file_path()) {
		if (addr.get_ip().empty()) {
			throw socket_error("IP address/file path is empty");
		}
	}

#ifdef NETKIT_DKP
	this->sockfd = ::socket(AF_INET, t == type::tcp ? SOCK_STREAM : SOCK_DGRAM, IPPROTO_IP);
#elifndef NETKIT_WINDOWS
    if (t != type::uds) {
        this->sockfd = ::socket(addr.is_ipv6() ? AF_INET6 : AF_INET,
                                                          t == type::tcp ? SOCK_STREAM : SOCK_DGRAM, 0);
    } else {
        this->sockfd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    }
#endif

    if (this->sockfd < 0) {
        throw socket_error("failed to create socket");
    }

    if (this->sockfd >= 0) {
        this->native_sync_sock::set_sock_opts(opts);
    } else {
        throw socket_error("cannot set options on invalid socket");
    }

    this->prep_sa();
}
#endif

netkit::sock::native::native_sync_sock::native_sync_sock(fd_t existing_fd, const sock::addr& peer, sock::type t, opt opts)
	: addr_(peer), type_(t), sockfd(existing_fd) {
	if (sockfd < 0) throw socket_error("invalid fd");
#ifdef NETKIT_WINDOWS
	if (this->sockfd != INVALID_SOCKET) {
#else
	if (this->sockfd >= 0) {
#endif
		this->native_sync_sock::set_sock_opts(opts);
	} else {
		throw socket_error("cannot set options on invalid socket");
	}

	this->prep_sa();
}

#ifdef NETKIT_WINDOWS
netkit::sock::native::native_sync_sock::native_sync_sock(const sock::addr& in_addr, sock::type t, opt opts)
    : addr_(in_addr), type_(t) {

    if (this->addr_.get_ip().empty() && !this->addr_.is_file_path()) {
        throw socket_error("IP address or file path is empty");
    }

    int domain = AF_UNIX;
    int sock_type = SOCK_STREAM;
    int protocol = 0;

    if (t != type::uds) {
        domain = this->addr_.is_ipv6() ? AF_INET6 : AF_INET;
        sock_type = (t == type::tcp) ? SOCK_STREAM : SOCK_DGRAM;
        protocol = (t == type::tcp) ? IPPROTO_TCP : IPPROTO_UDP;
    } else {
        protocol = 0;
    }

    this->sockfd = socket(domain, sock_type, protocol);
    if (this->sockfd == INVALID_SOCKET) {
        throw socket_error("Failed to create socket");
    }

    this->native_sync_sock::set_sock_opts(opts);
    this->prep_sa();
}
#endif
#ifdef NETKIT_UNIX
netkit::sock::native::native_sync_sock::~native_sync_sock() {
    if (this->sockfd == -1) {
        return;
    }
    if (::close(this->sockfd) < 0) {
        ;
    }
}
#endif
#ifdef NETKIT_WINDOWS
netkit::sock::native::native_sync_sock::~native_sync_sock() {
    if (this->sockfd == INVALID_SOCKET) {
        return;
    }

    if (::closesocket(this->sockfd) == SOCKET_ERROR) {
        return;
    }

    this->sockfd = INVALID_SOCKET;
}
#endif

netkit::sock::addr& netkit::sock::native::native_sync_sock::get_addr() {
    return this->addr_;
}

const netkit::sock::addr& netkit::sock::native::native_sync_sock::get_addr() const {
    return this->addr_;
}

std::size_t netkit::sock::native::native_sync_sock::send(const void* buf, size_t len) {
	return ::send(this->sockfd, static_cast<const char*>(buf), len, 0);
}

std::size_t netkit::sock::native::native_sync_sock::recv(void* buf, std::size_t len) {
	return ::recv(this->sockfd, static_cast<char*>(buf), len, 0);
}

#ifdef NETKIT_UNIX
void netkit::sock::native::native_sync_sock::close() noexcept {
    if (this->sockfd == -1) {
        return;
    }

    (void)::close(this->sockfd);
    this->sockfd = -1;
}
#elifdef NETKIT_WINDOWS
void netkit::sock::native::native_sync_sock::close() noexcept {
    if (this->sockfd == INVALID_SOCKET) {
        return;
    }

    ::shutdown(this->sockfd, SD_BOTH);

    if (::closesocket(this->sockfd) != 0) {
        ;
    }

    sockfd = INVALID_SOCKET;
}
#endif
[[nodiscard]] netkit::sock::addr netkit::sock::native::native_sync_sock::get_peer() const {
#ifdef NETKIT_DKP
	if (!this->has_peer) {
		throw netkit::socket_error("peer not known");
	}

	char ip_str[INET6_ADDRSTRLEN]{};
	uint16_t port = 0;

	if (this->peer_addr.ss_family == AF_INET) {
		auto* addr_in = (sockaddr_in*)&this->peer_addr;
		inet_ntop(AF_INET, &addr_in->sin_addr, ip_str, sizeof(ip_str));
		port = ntohs(addr_in->sin_port);
	} else {
		throw netkit::ip_error("unsupported address family (Wii = IPv4 only)");
	}

	return netkit::sock::addr{
		ip_str,
		port, netkit::sock::addr_type::ipv4
	};
#else
    return netkit::sock::native::get_peer(this->sockfd);
#endif
}
netkit::sock::fd_t netkit::sock::native::native_sync_sock::native_handle() const {
	return this->sockfd;
}