/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol
 * abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file async_sock.cpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Implementation of the asynchronous socket class.
 */

#include <cstring>
#include <iostream>
#include <chrono>

#include <netkit/except.hpp>
#include <netkit/sock/async_sock.hpp>

#ifdef NETKIT_LINUX

#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

namespace netkit::sock {
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

const sockaddr* netkit::sock::async_sock::get_sa() const {
    return reinterpret_cast<const sockaddr*>(&sa_storage);
}

socklen_t netkit::sock::async_sock::get_sa_len() const {
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

void netkit::sock::async_sock::prep_sa() {
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

void netkit::sock::async_sock::set_sock_opts(opt opts) {
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

netkit::sock::async_sock::async_sock(netkit::io::io_context& ctx, const sock::addr& addr, sock::type t, opt opts)
	: addr_(addr), type_(t), context_(ctx) {
    this->sockfd = -1;

	if (!addr.is_file_path()) {
		if (addr.get_ip().empty()) {
			throw socket_error("IP address/file path is empty");
		}
	}

    if (t != type::uds) {
        this->sockfd = ::socket(addr.is_ipv6() ? AF_INET6 : AF_INET,
                                                          t == type::tcp ? SOCK_STREAM : SOCK_DGRAM, 0);
    } else {
        this->sockfd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    }

    if (this->sockfd < 0) {
        throw socket_error("failed to create socket");
    }

    if (this->sockfd >= 0) {
        this->async_sock::set_sock_opts(opts);
    } else {
        throw socket_error("cannot set options on invalid socket");
    }

    this->prep_sa();
}

netkit::sock::async_sock::async_sock(netkit::io::io_context& ctx, int existing_fd, const sock::addr& peer, sock::type t, opt opts)
    : addr_(peer), type_(t), sockfd(existing_fd), context_(ctx) {
    if (sockfd < 0) throw socket_error("invalid fd");
    if (this->sockfd >= 0) {
        this->async_sock::set_sock_opts(opts);
    } else {
        throw socket_error("cannot set options on invalid socket");
    }

    this->prep_sa();
}

netkit::sock::async_sock::~async_sock() {
    if (this->sockfd == -1) {
        return;
    }
    if (::close(this->sockfd) < 0) {
        ;
    }
}

netkit::sock::addr& netkit::sock::async_sock::get_addr() {
    return this->addr_;
}

const netkit::sock::addr& netkit::sock::async_sock::get_addr() const {
    return this->addr_;
}

netkit::io::task<void> netkit::sock::async_sock::connect() {
	int ret = ::connect(
		this->sockfd,
		this->get_sa(),
		this->get_sa_len()
	);

	if (ret == 0) {
		co_return;
	}


	if (errno != EINPROGRESS) {
		throw netkit::socket_error(
			"failed to connect to server"
		);
	}

	co_await this->context_.wait_writable(this->sockfd);

	int error = 0;
	socklen_t len = sizeof(error);

	if (getsockopt(
			this->sockfd,
			SOL_SOCKET,
			SO_ERROR,
			&error,
			&len
		) < 0) {
		throw netkit::socket_error(
			"getsockopt(SO_ERROR) failed"
		);
	}

	if (error != 0) {
		errno = error;

		throw netkit::socket_error(
			"failed to connect to server"
		);
	}

	co_return;
}

void netkit::sock::async_sock::bind() {
	auto ret = ::bind(
		this->sockfd,
		this->get_sa(),
		this->get_sa_len()
	);

	if (ret < 0) {
		throw socket_error("failed to bind socket");
	}

	this->bound = true;
}

void netkit::sock::async_sock::unbind() {
    if (this->bound) {
        if (::close(this->sockfd) < 0) {
            throw socket_error("failed to unbind socket");
        }
        this->bound = false;
    }
}

void netkit::sock::async_sock::listen(int backlog) {
    if (::listen(this->sockfd, backlog == -1 ? SOMAXCONN : backlog) < 0) {
        throw socket_error("failed to listen on socket");
    }
}

void netkit::sock::async_sock::listen() {
	this->listen(-1);
}

netkit::io::task<std::unique_ptr<netkit::sock::basic_async_sock>>
netkit::sock::async_sock::accept() {
	while (true) {
		sockaddr_storage client_addr{};
		socklen_t addr_len = sizeof(client_addr);

		// TODO: use fcntl and get bsd support
		// it is just a waste of time right now since we are using epoll anyway
		fd_t client_sockfd = ::accept4(
			this->sockfd,
			reinterpret_cast<sockaddr*>(&client_addr),
			&addr_len,
			SOCK_NONBLOCK
		);

		if (client_sockfd >= 0) {
			if (this->type_ == type::uds) {
				co_return std::make_unique<async_sock>(
					this->context_,
					client_sockfd,
					sock::addr(
						reinterpret_cast<const sockaddr_un*>(&client_addr)->sun_path
					),
					this->type_
				);
			}

			auto peer = sock::get_peer(client_sockfd);

			co_return std::make_unique<async_sock>(
				this->context_,
				client_sockfd,
				peer,
				this->type_
			);
		}

		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			co_await this->context_.wait_readable(this->sockfd);
			continue;
		}

		throw socket_error(
			"failed to accept connection: " +
			std::string(strerror(errno))
		);
	}
}

netkit::io::task<std::size_t> netkit::sock::async_sock::send(const void* buf, size_t len) {
	size_t total_sent = 0;
	const char* data = static_cast<const char*>(buf);

	while (total_sent < len) {
		ssize_t sent = ::send(
			this->sockfd,
			data + total_sent,
			len - total_sent,
			0
		);

		if (sent > 0) {
			total_sent += sent;
			continue;
		}

		if (sent == 0) {
			co_return total_sent;
		}

		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			co_await this->context_.wait_writable(
				this->sockfd
			);

			continue;
		}

		throw socket_error(
			"failed to send: " +
			std::string(strerror(errno))
		);
	}

	co_return total_sent;
}

netkit::io::task<std::size_t> netkit::sock::async_sock::recv(void* buf, size_t size) {
	for (;;) {
		auto n = ::recv(
			this->sockfd,
			static_cast<char*>(buf),
			size,
			0
		);

		if (n > 0) {
			co_return static_cast<size_t>(n);
		}

		if (n == 0) {
			co_return 0;
		}

		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			co_await context_.wait_readable(sockfd);
			continue;
		}

		if (errno == EINTR)
			continue;

		throw socket_error(
			"recv failed: " + std::string(strerror(errno))
		);
	}
}

void netkit::sock::async_sock::close() {
	if (this->sockfd == -1)
		return;

	::close(this->sockfd);
	this->sockfd = -1;
}

[[nodiscard]] netkit::sock::addr netkit::sock::async_sock::get_peer() const {
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
    return sock::get_peer(this->sockfd);
#endif
}
netkit::sock::fd_t netkit::sock::async_sock::native_handle() const {
	return this->sockfd;
}

#endif