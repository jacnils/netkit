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
#include <fcntl.h>
#include <netkit/except.hpp>

#include <netkit/socket/native/peer_helper.hpp>

#ifdef NETKIT_UNIX
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/un.h>
#elifdef NETKIT_WINDOWS
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
#ifdef NETKIT_UNIX
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
#elifdef NETKIT_WINDOWS
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
#endif
}

netkit::sock::native::native_async_sock::native_async_sock(netkit::io::io_context& ctx, const sock::addr& addr, sock::type t, opt opts)
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
        this->native_async_sock::set_sock_opts(opts);
    } else {
        throw socket_error("cannot set options on invalid socket");
    }

    this->prep_sa();
}

netkit::sock::native::native_async_sock::native_async_sock(netkit::io::io_context& ctx, fd_t existing_fd, const sock::addr& peer, sock::type t, opt opts)
    : addr_(peer), type_(t), sockfd(existing_fd), context_(ctx) {
    if (sockfd < 0) throw socket_error("invalid fd");
    if (this->sockfd >= 0) {
        this->native_async_sock::set_sock_opts(opts);
    } else {
        throw socket_error("cannot set options on invalid socket");
    }

    this->prep_sa();
}

netkit::sock::native::native_async_sock::~native_async_sock() {
    if (this->sockfd == -1) {
        return;
    }
    if (::close(this->sockfd) < 0) {
        ;
    }
}

netkit::sock::addr& netkit::sock::native::native_async_sock::get_addr() {
    return this->addr_;
}

const netkit::sock::addr& netkit::sock::native::native_async_sock::get_addr() const {
    return this->addr_;
}

#ifdef NETKIT_UNIX
netkit::io::task<void> netkit::sock::native::native_async_sock::connect() {
	int ret = ::connect(
		this->sockfd,
		this->get_sa(),
		this->get_sa_len()
	);

	if (ret == 0) {
		co_return;
	}

	if (errno != EINPROGRESS) {
		throw netkit::socket_error("failed to connect to server");
	}

	co_await this->context_.wait_writable(this->sockfd);

	int error = 0;
	socklen_t len = sizeof(error);

	if (getsockopt(this->sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
		throw netkit::socket_error("getsockopt(SO_ERROR) failed");
	}

	if (error != 0) {
		errno = error;

		throw netkit::socket_error("failed to connect to server");
	}

	co_return;
}
#elifdef NETKIT_WINDOWS
netkit::io::task<void>
netkit::sock::native::native_async_sock::connect() {
	int ret = ::connect(
		this->sockfd,
		this->get_sa(),
		this->get_sa_len()
	);

	if (ret == 0) {
		co_return;
	}

	int err = WSAGetLastError();

	if (err != WSAEWOULDBLOCK && err != WSAEINPROGRESS) {
		throw netkit::socket_error("failed to connect to server");
	}

	co_await this->context_.wait_writable(this->sockfd);

	int error = 0;
	int len = sizeof(error);

	if (getsockopt(
			this->sockfd,
			SOL_SOCKET,
			SO_ERROR,
			reinterpret_cast<char*>(&error),
			&len
		) == SOCKET_ERROR) {
		throw netkit::socket_error("getsockopt(SO_ERROR) failed");
		}

	if (error != 0) {
		throw netkit::socket_error(
			"failed to connect to server: " + std::to_string(error)
		);
	}

	co_return;
}
#endif
#ifdef NETKIT_UNIX
netkit::io::task<std::size_t> netkit::sock::native::native_async_sock::send(const void* buf, size_t len) {
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

		throw socket_error("failed to send: " + std::string(strerror(errno)));
	}

	co_return total_sent;
}
#elifdef NETKIT_WINDOWS
netkit::io::task<std::size_t>
netkit::sock::native::native_async_sock::send(const void* buf, size_t len) {
	size_t total_sent = 0;
	const char* data = static_cast<const char*>(buf);

	while (total_sent < len) {
		int sent = ::send(
			this->sockfd,
			data + total_sent,
			static_cast<int>(len - total_sent),
			0
		);

		if (sent > 0) {
			total_sent += sent;
			continue;
		}

		if (sent == 0) {
			co_return total_sent;
		}

		int err = WSAGetLastError();

		if (err == WSAEWOULDBLOCK) {
			co_await this->context_.wait_writable(this->sockfd);
			continue;
		}

		throw socket_error("failed to send: " + std::to_string(err));
	}

	co_return total_sent;
}
#endif
#ifdef NETKIT_UNIX
netkit::io::task<std::size_t> netkit::sock::native::native_async_sock::recv(void* buf, size_t size) {
	for (;;) {
		auto n = ::recv(this->sockfd, buf, size, 0);

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
#elifdef NETKIT_WINDOWS
netkit::io::task<std::size_t>
netkit::sock::native::native_async_sock::recv(void* buf, size_t size) {
	for (;;) {
		int n = ::recv(
			this->sockfd,
			static_cast<char*>(buf),
			static_cast<int>(size),
			0
		);

		if (n > 0) {
			co_return static_cast<size_t>(n);
		}

		if (n == 0) {
			co_return 0;
		}

		int err = WSAGetLastError();

		if (err == WSAEWOULDBLOCK) {
			co_await context_.wait_readable(sockfd);
			continue;
		}

		if (err == WSAEINTR) {
			continue;
		}

		throw socket_error(
			"recv failed: " + std::to_string(err)
		);
	}
}
#endif

void netkit::sock::native::native_async_sock::close() noexcept {
	if (this->sockfd == -1)
		return;

	::close(this->sockfd);
	this->sockfd = -1;
}

[[nodiscard]] netkit::sock::addr netkit::sock::native::native_async_sock::get_peer() const {
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
		throw netkit::ip_error("unsupported address family");
	}

	return netkit::sock::addr{
		ip_str,
		port, netkit::sock::addr_type::ipv4
	};
#else
    return native::get_peer(this->sockfd);
#endif
}
netkit::sock::fd_t netkit::sock::native::native_async_sock::native_handle() const {
	return this->sockfd;
}