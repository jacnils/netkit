#include <cstring>
#include <fcntl.h>
#include <memory>
#include <netkit/except.hpp>
#include <netkit/socket/native/native_sync_listener.hpp>
#include <netkit/socket/native/native_sync_sock.hpp>
#include <netkit/socket/native/peer_helper.hpp>

#ifdef NETKIT_UNIX
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/un.h>
#elifdef NETKIT_WINDOWS
#include <afunix.h>
#endif

#include <unistd.h>

void netkit::sock::native::native_sync_listener::set_sock_opts(opt opts) const {
#ifdef NETKIT_UNIX
	if (opts & opt::reuse_addr) {
		::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &opts, sizeof(opts));
	} else if (opts & opt::no_reuse_addr) {
		::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, nullptr, 0);
	}
	if (opts & opt::no_delay) {
		::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &opts, sizeof(opts));
	}
	if (opts & opt::keep_alive) {
		::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &opts, sizeof(opts));
	} else if (opts & opt::no_keep_alive) {
		::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, nullptr, 0);
	}
	if (opts & opt::no_blocking) {
		int flags = fcntl(this->sockfd_, F_GETFL, 0);
		if (flags < 0) {
			::close(this->sockfd_);
			throw socket_error("failed to get socket flags");
		}
		if (fcntl(this->sockfd_, F_SETFL, flags | O_NONBLOCK) < 0) {
			::close(this->sockfd_);
			throw socket_error("failed to set socket to non-blocking mode");
		}
	} else if (opts & opt::blocking) {
		int flags = fcntl(this->sockfd_, F_GETFL, 0);
		if (flags < 0) {
			::close(this->sockfd_);
			throw socket_error("failed to get socket flags");
		}
		if (fcntl(this->sockfd_, F_SETFL, flags & ~O_NONBLOCK) < 0) {
			::close(this->sockfd_);
			throw socket_error("failed to set socket to blocking mode");
		}
	}
#elifdef NETKIT_WINDOWS
    if (opts & opt::reuse_addr) {
        BOOL optval = TRUE;
        if (setsockopt(this->sockfd_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&optval), sizeof(optval)) == SOCKET_ERROR) {
            closesocket(this->sockfd_);
            throw socket_error("failed to set SO_REUSEADDR");
        }
    } else if (opts & opt::no_reuse_addr) {
        BOOL optval = FALSE;
        if (setsockopt(this->sockfd_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&optval), sizeof(optval)) == SOCKET_ERROR) {
            closesocket(this->sockfd_);
            throw socket_error("failed to clear SO_REUSEADDR");
        }
    }
	if ((opts & opt::no_delay) && type_ == type::tcp) {
        BOOL optval = TRUE;
        if (setsockopt(this->sockfd_, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&optval), sizeof(optval)) == SOCKET_ERROR) {
            closesocket(this->sockfd_);
            throw socket_error("failed to set TCP_NODELAY");
        }
    }
    if (opts & opt::keep_alive) {
        BOOL optval = TRUE;
        if (setsockopt(this->sockfd_, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char*>(&optval), sizeof(optval)) == SOCKET_ERROR) {
            closesocket(this->sockfd_);
            throw socket_error("failed to set SO_KEEPALIVE");
        }
    } else if (opts & opt::no_keep_alive) {
        BOOL optval = FALSE;
        if (setsockopt(this->sockfd_, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char*>(&optval), sizeof(optval)) == SOCKET_ERROR) {
            closesocket(this->sockfd_);
            throw socket_error("failed to clear SO_KEEPALIVE");
        }
    }
    if (opts & opt::no_blocking) {
        u_long mode = 1;
        if (ioctlsocket(this->sockfd_, FIONBIO, &mode) == SOCKET_ERROR) {
            closesocket(this->sockfd_);
            throw socket_error("failed to set socket to non-blocking mode");
        }
    } else if (opts & opt::blocking) {
        u_long mode = 0;
        if (ioctlsocket(this->sockfd_, FIONBIO, &mode) == SOCKET_ERROR) {
            closesocket(this->sockfd_);
            throw socket_error("failed to set socket to blocking mode");
        }
    }
#endif
}

const sockaddr* netkit::sock::native::native_sync_listener::get_sa() const {
	return reinterpret_cast<const sockaddr*>(&sa_storage_);
}

socklen_t netkit::sock::native::native_sync_listener::get_sa_len() const {
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

void netkit::sock::native::native_sync_listener::prep_sa() {
	memset(&sa_storage_, 0, sizeof(sa_storage_));

	if (addr_.is_ipv4()) {
		auto* sa4 = reinterpret_cast<sockaddr_in*>(&sa_storage_);
		sa4->sin_family = AF_INET;
		sa4->sin_port = htons(addr_.get_port());
		if (inet_pton(AF_INET, addr_.get_ip().c_str(), &sa4->sin_addr) <= 0) {
			throw parsing_error("invalid IPv4 address");
		}
	} else if (addr_.is_ipv6()) {
		auto* sa6 = reinterpret_cast<sockaddr_in6*>(&sa_storage_);
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
		auto* sa_un = reinterpret_cast<sockaddr_un*>(&sa_storage_);
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

netkit::sock::native::native_sync_listener::native_sync_listener(const addr& address, type t, opt opts)
: addr_(address), type_(t), opts_(opts) {
	sockfd_ = ::socket(
		addr_.is_ipv6()
			? AF_INET6
			: AF_INET,
		SOCK_STREAM,
		0
	);

	if (sockfd_ < 0)
		throw socket_error(
			"failed creating socket"
		);

	set_sock_opts(opts_);
	prep_sa();
}

void netkit::sock::native::native_sync_listener::bind() {
	if (::bind(sockfd_, get_sa(), get_sa_len()) < 0) {
		throw socket_error(
			"bind failed"
		);
	}

	bound_ = true;
}

void netkit::sock::native::native_sync_listener::unbind() {
	this->close();
}

void netkit::sock::native::native_sync_listener::listen(int backlog) {
	if (!bound_) throw socket_error("listener not bound");

	if (::listen(this->sockfd_, backlog == -1 ? SOMAXCONN : backlog) < 0) {
		throw socket_error("failed to listen on socket");
	}

	listening_ = true;
}

void netkit::sock::native::native_sync_listener::listen() {
	this->listen(-1);
}

std::unique_ptr<netkit::sock::native::basic_native_sync_sock>
netkit::sock::native::native_sync_listener::accept() {
	sockaddr_storage client_addr{};
	socklen_t addr_len = sizeof(client_addr);

	auto client_sockfd = ::accept(this->sockfd_, reinterpret_cast<sockaddr*>(&client_addr), &addr_len);
	if (client_sockfd < 0) {
		throw socket_error("failed to accept connection: " + std::string(std::strerror(errno)));
	}

	if (this->type_ == type::uds) {
		return std::make_unique<native_sync_sock>(client_sockfd, sock::addr(reinterpret_cast<const sockaddr_un*>(&client_addr)->sun_path), this->type_);
	}

	if (this->type_ == type::uds) {
		return std::make_unique<native_sync_sock>(client_sockfd, sock::addr(reinterpret_cast<const sockaddr_un*>(&client_addr)->sun_path), this->type_);
	}

	auto peer = native::get_peer(client_sockfd);
	return std::make_unique<native_sync_sock>(client_sockfd, peer, this->type_);
}

void netkit::sock::native::native_sync_listener::close() noexcept {
	if (sockfd_ != INVALID_SOCKET) {
		::close(sockfd_);
		sockfd_ = INVALID_SOCKET;
	}

	this->bound_ = this->listening_ = false;
}

netkit::sock::fd_t netkit::sock::native::native_sync_listener::native_handle() const{
	return sockfd_;
}

const netkit::sock::addr& netkit::sock::native::native_sync_listener::get_local_endpoint() const {
	return addr_;
}

