#include <netkit/platform/socket.hpp>

#include <cstring>
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
#include <fcntl.h>
#elif defined(NETKIT_WINDOWS)
#include <afunix.h>
#endif

#include <unistd.h>

void netkit::sock::native::native_sync_listener::set_sock_opts(opt opts) const {
	netkit::platform::set_sock_opts(this->sockfd_, opts);
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
	sockfd_ = platform::socket(addr_.is_ipv6() ? AF_INET6 : AF_INET, SOCK_STREAM, 0);

	if (!platform::valid_socket(sockfd_))
		throw socket_error("failed creating socket");

	set_sock_opts(opts_);
	prep_sa();
}

void netkit::sock::native::native_sync_listener::bind() {
	if (platform::bind(sockfd_, get_sa(), get_sa_len()) < 0) {
		throw socket_error("bind failed");
	}

	bound_ = true;
}

void netkit::sock::native::native_sync_listener::unbind() {
	this->close();
}

void netkit::sock::native::native_sync_listener::listen(int backlog) {
	if (!bound_) throw socket_error("listener not bound");

	if (platform::listen(this->sockfd_, backlog == -1 ? SOMAXCONN : backlog) < 0) {
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

	auto client_sockfd = platform::accept(this->sockfd_, reinterpret_cast<sockaddr*>(&client_addr), &addr_len);
	if (!platform::valid_socket(client_sockfd)) {
		throw socket_error("failed to accept connection: " + std::string(std::strerror(errno)));
	}

#ifndef NETKIT_DKP
	if (this->type_ == type::uds) {
		return std::make_unique<native_sync_sock>(client_sockfd, sock::addr(reinterpret_cast<const sockaddr_un*>(&client_addr)->sun_path), this->type_);
	}
#endif

	auto peer = native::get_peer(client_sockfd);
	return std::make_unique<native_sync_sock>(client_sockfd, peer, this->type_);
}

void netkit::sock::native::native_sync_listener::close() noexcept {
	if (platform::valid_socket(sockfd_)) {
		platform::close_socket(sockfd_);
		this->bound_ = this->listening_ = false;
	}
}

netkit::sock::fd_t netkit::sock::native::native_sync_listener::native_handle() const{
	return sockfd_;
}

const netkit::sock::addr& netkit::sock::native::native_sync_listener::get_local_endpoint() const {
	return addr_;
}

