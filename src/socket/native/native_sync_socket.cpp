/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file native_sync_socket.cpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Implementation of the synchronous socket class.
 */
#include <netkit/socket/native/native_sync_sock.hpp>
#include <cstring>
#include <iostream>
#include <netkit/except.hpp>
#include <netkit/socket/sock_peer.hpp>

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

#ifdef NETKIT_DKP
#define NETKIT_SELECT ::net_select
#else
#define NETKIT_SELECT select
#endif

#ifndef NETKIT_DKP
namespace netkit::sock::native {
netkit::sock::addr get_peer(netkit::sock::fd_t sockfd) {
	sockaddr_storage addr_storage{};
	socklen_t addr_len = sizeof(addr_storage);

	if (getpeername(sockfd, reinterpret_cast<sockaddr*>(&addr_storage), &addr_len) < 0) {
		throw netkit::socket_error("getpeername() failed: " + std::string(strerror(errno)));
	}

	char ip_str[INET6_ADDRSTRLEN] = {0};
	uint16_t port = 0;

	if (addr_storage.ss_family == AF_INET) {
		auto* addr_in = reinterpret_cast<sockaddr_in*>(&addr_storage);
		inet_ntop(AF_INET, &(addr_in->sin_addr), ip_str, sizeof(ip_str));
		port = ntohs(addr_in->sin_port);
	} else if (addr_storage.ss_family == AF_INET6) {
		auto* addr_in6 = reinterpret_cast<sockaddr_in6*>(&addr_storage);
		inet_ntop(AF_INET6, &(addr_in6->sin6_addr), ip_str, sizeof(ip_str));
		port = ntohs(addr_in6->sin6_port);
	} else {
		throw netkit::ip_error("unsupported address family");
	}

	netkit::sock::addr addr{};
	addr.ip = ip_str;
	addr.port = port;
	addr.type = (addr_storage.ss_family == AF_INET) ? netkit::sock::addr_type::ipv4 : netkit::sock::addr_type::ipv6;

	return addr;
}
}
#endif

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
#ifdef NETKIT_UNIX
void netkit::sock::native::native_sync_sock::set_sock_opts(opt opts) {
    if (opts & opt::reuse_addr) {
        ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opts, sizeof(opts));
    } else if (opts & opt::no_reuse_addr) {
        ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, nullptr, 0);
    }
    if (opts & opt::no_delay) {
        ::setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &opts, sizeof(opts));
    }
    if (opts & opt::keep_alive) {
        ::setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &opts, sizeof(opts));
    } else if (opts & opt::no_keep_alive) {
        ::setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, nullptr, 0);
    }
    if (opts & opt::no_blocking) {
        int flags = fcntl(this->sockfd, F_GETFL, 0);
        if (flags < 0) {
            ::close(this->sockfd);
            throw socket_error("failed to get socket flags");
        }
        if (fcntl(this->sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
            ::close(this->sockfd);
            throw socket_error("failed to set socket to non-blocking mode");
        }
    } else if (opts & opt::blocking) {
        int flags = fcntl(this->sockfd, F_GETFL, 0);
        if (flags < 0) {
            ::close(this->sockfd);
            throw socket_error("failed to get socket flags");
        }
        if (fcntl(this->sockfd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
            ::close(this->sockfd);
            throw socket_error("failed to set socket to blocking mode");
        }
    }
}
#endif
#ifdef NETKIT_WINDOWS
void netkit::sock::native::native_sync_sock::set_sock_opts(opt opts) {
    if (opts & opt::reuse_addr) {
        BOOL optval = TRUE;
        if (setsockopt(this->sockfd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&optval), sizeof(optval)) == SOCKET_ERROR) {
            closesocket(this->sockfd);
            throw socket_error("failed to set SO_REUSEADDR");
        }
    } else if (opts & opt::no_reuse_addr) {
        BOOL optval = FALSE;
        if (setsockopt(this->sockfd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&optval), sizeof(optval)) == SOCKET_ERROR) {
            closesocket(this->sockfd);
            throw socket_error("failed to clear SO_REUSEADDR");
        }
    }
	if ((opts & opt::no_delay) && type_ == type::tcp) {
        BOOL optval = TRUE;
        if (setsockopt(this->sockfd, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&optval), sizeof(optval)) == SOCKET_ERROR) {
            closesocket(this->sockfd);
            throw socket_error("failed to set TCP_NODELAY");
        }
    }
    if (opts & opt::keep_alive) {
        BOOL optval = TRUE;
        if (setsockopt(this->sockfd, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char*>(&optval), sizeof(optval)) == SOCKET_ERROR) {
            closesocket(this->sockfd);
            throw socket_error("failed to set SO_KEEPALIVE");
        }
    } else if (opts & opt::no_keep_alive) {
        BOOL optval = FALSE;
        if (setsockopt(this->sockfd, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char*>(&optval), sizeof(optval)) == SOCKET_ERROR) {
            closesocket(this->sockfd);
            throw socket_error("failed to clear SO_KEEPALIVE");
        }
    }
    if (opts & opt::no_blocking) {
        u_long mode = 1;
        if (ioctlsocket(this->sockfd, FIONBIO, &mode) == SOCKET_ERROR) {
            closesocket(this->sockfd);
            throw socket_error("failed to set socket to non-blocking mode");
        }
    } else if (opts & opt::blocking) {
        u_long mode = 0;
        if (ioctlsocket(this->sockfd, FIONBIO, &mode) == SOCKET_ERROR) {
            closesocket(this->sockfd);
            throw socket_error("failed to set socket to blocking mode");
        }
    }
}
#endif

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
#else
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

netkit::sock::native::native_sync_sock::native_sync_sock(int existing_fd, const sock::addr& peer, sock::type t, opt opts)
    : addr_(peer), type_(t), sockfd(existing_fd) {
    if (sockfd < 0) throw socket_error("invalid fd");
    if (this->sockfd >= 0) {
        this->native_sync_sock::set_sock_opts(opts);
    } else {
        throw socket_error("cannot set options on invalid socket");
    }

    this->prep_sa();
}
#endif
#ifdef NETKIT_WINDOWS
netkit::sock::native::native_sync_sock::sync_sock(const sock::addr& in_addr, sock::type t, opt opts)
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

    this->sync_sock::set_sock_opts(opts);
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
#ifdef NETKIT_UNIX
void netkit::sock::native::native_sync_sock::connect() {
    if (::connect(this->sockfd, this->get_sa(), this->get_sa_len()) < 0) {
        throw netkit::socket_error("failed to connect to server");
    }

#ifdef NETKIT_DKP
	std::memcpy(&this->peer_addr, this->get_sa(), this->get_sa_len());
	this->has_peer = true;
#endif
}
#endif
#ifdef NETKIT_WINDOWS
void netkit::sock::native::native_sync_sock::connect() {
    if (::connect(this->sockfd, this->get_sa(), this->get_sa_len()) == SOCKET_ERROR) {
        throw socket_error("failed to connect to server");
    }
}
#endif
#ifdef NETKIT_UNIX
void netkit::sock::native::native_sync_sock::bind() {
    this->bound = true;

    auto ret = ::bind(this->sockfd, this->get_sa(), this->get_sa_len());

    if (ret < 0) {
        throw socket_error("failed to bind socket: " + std::to_string(ret));
    }
}
#endif
#ifdef NETKIT_WINDOWS
void netkit::sock::native::native_sync_sock::bind() {
    this->bound = true;

    int result = ::bind(this->sockfd, this->get_sa(), this->get_sa_len());

    if (result == SOCKET_ERROR) {
        int err = WSAGetLastError();
        throw socket_error("failed to bind socket, error code: " + std::to_string(err));
    }
}
#endif
#ifdef NETKIT_UNIX
void netkit::sock::native::native_sync_sock::unbind() {
    if (this->bound) {
        if (::close(this->sockfd) < 0) {
            throw socket_error("failed to unbind socket");
        }
        this->bound = false;
    }
}
#endif
#ifdef NETKIT_WINDOWS
void netkit::sock::native::native_sync_sock::unbind() {
    if (this->bound) {
        if (::closesocket(this->sockfd) == SOCKET_ERROR) {
            int err = WSAGetLastError();
            throw socket_error("failed to close socket, error code: " + std::to_string(err));
        }
        this->bound = false;
        this->sockfd = INVALID_SOCKET;
    }
}
#endif
#ifdef NETKIT_UNIX
void netkit::sock::native::native_sync_sock::listen(int backlog) {
    if (::listen(this->sockfd, backlog == -1 ? SOMAXCONN : backlog) < 0) {
        throw socket_error("failed to listen on socket");
    }
}
#endif
#ifdef NETKIT_WINDOWS
void netkit::sock::native::native_sync_sock::listen(int backlog) {
    if (::listen(this->sockfd, backlog == -1 ? SOMAXCONN : backlog) == SOCKET_ERROR) {
        int err = WSAGetLastError();
        throw socket_error("failed to listen socket, error code: " + std::to_string(err));
    }
}
#endif
void netkit::sock::native::native_sync_sock::listen() {
    listen(-1);
}
#ifdef NETKIT_UNIX
std::unique_ptr<netkit::sock::native::basic_native_sync_sock> netkit::sock::native::native_sync_sock::accept() {
    sockaddr_storage client_addr{};
    socklen_t addr_len = sizeof(client_addr);

    int client_sockfd = ::accept(this->sockfd, reinterpret_cast<sockaddr*>(&client_addr), &addr_len);
    if (client_sockfd < 0) {
        throw socket_error("failed to accept connection: " + std::string(strerror(errno)));
    }

#ifndef NETKIT_DKP
	if (this->type_ == type::uds) {
		return std::make_unique<native_sync_sock>(client_sockfd, sock::addr(reinterpret_cast<const sockaddr_un*>(&client_addr)->sun_path), this->type_);
	}

	auto peer = sock::get_peer(client_sockfd);
	return std::make_unique<native_sync_sock>(client_sockfd, peer, this->type_);
#else // fuck this code
	char ip_str[INET6_ADDRSTRLEN]{};
	uint16_t port = 0;

	if (client_addr.ss_family == AF_INET) {
		auto* addr_in = reinterpret_cast<sockaddr_in*>(&client_addr);
		inet_ntop(AF_INET, &addr_in->sin_addr, ip_str, sizeof(ip_str));
		port = ntohs(addr_in->sin_port);
	} else {
		throw ip_error("unsupported address family");
	}

	sock::addr peer{
		ip_str,
		port,
		addr_type::ipv4
	};

	auto sock_ptr = std::make_unique<native_sync_sock>(client_sockfd, peer, this->type_);

	std::memcpy(&sock_ptr->peer_addr, &client_addr, addr_len);
	sock_ptr->has_peer = true;

	return sock_ptr;

#endif

}
#endif
#ifdef NETKIT_WINDOWS
std::unique_ptr<netkit::sock::native::basic_native_sync_sock> netkit::sock::native::native_sync_sock::accept() {
    sockaddr_storage client_addr{};
    int addr_len = sizeof(client_addr);

    SOCKET client_sockfd = ::accept(this->sockfd, reinterpret_cast<sockaddr*>(&client_addr), &addr_len);
    if (client_sockfd == INVALID_SOCKET) {
        int err = WSAGetLastError();
        throw socket_error("failed to accept connection, error code: " + std::to_string(err));
    }

    auto peer = sock::get_peer(client_sockfd);
    auto handle = std::make_unique<native_sync_sock>(peer, this->type_);
    handle->sockfd = client_sockfd;

    return handle;
}
#endif

std::size_t netkit::sock::native::native_sync_sock::send(const void* buf, size_t len) {
	return ::send(this->sockfd, buf, len, 0);
}

std::size_t netkit::sock::native::native_sync_sock::recv(void* buf, std::size_t len) {
	return ::recv(this->sockfd, buf, len, 0);
}

#ifdef NETKIT_UNIX
void netkit::sock::native::native_sync_sock::close() {
    if (this->sockfd == -1) {
        return;
    }

    (void)::close(this->sockfd);
    this->sockfd = -1;
}
#elifdef NETKIT_WINDOWS
void netkit::sock::native::native_sync_sock::close() {
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
    return native::get_peer(this->sockfd);
#endif
}
netkit::sock::fd_t netkit::sock::native::native_sync_sock::native_handle() const {
	return this->sockfd;
}