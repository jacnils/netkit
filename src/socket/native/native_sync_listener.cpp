#include <netkit/platform/socket.hpp>

#include <cstring>
#include <memory>
#include <netkit/except.hpp>
#include <netkit/socket/native/native_sync_listener.hpp>
#include <netkit/socket/native/native_sync_socket.hpp>
#include <netkit/socket/native/peer_helper.hpp>

#if defined(NETKIT_UNIX) && !defined(NETKIT_DKP)
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#elif defined(NETKIT_WINDOWS)
#include <afunix.h>
#elif defined(NETKIT_DKP)
// nothing
#endif

void netkit::socket::native::native_sync_listener::set_sock_opts(opt opts) const {
	netkit::platform::set_sock_opts(this->sockfd_, opts);
}

netkit::socket::native::native_sync_listener::native_sync_listener(const addr& address, type t, opt opts)
: addr_(address), type_(t), opts_(opts) {
#ifndef NETKIT_DKP
	if (this->type_ == type::uds) {
		sockfd_ = platform::socket(AF_UNIX, SOCK_STREAM, 0);
	} else {
		sockfd_ = platform::socket(addr_.is_ipv6() ? AF_INET6 : AF_INET, t == type::tcp ? SOCK_STREAM : SOCK_DGRAM, 0);
	}
#else
	sockfd_ = platform::socket(addr_.is_ipv6() ? AF_INET6 : AF_INET, t == type::tcp ? SOCK_STREAM : SOCK_DGRAM, 0);
#endif

	if (!platform::valid_socket(sockfd_))
		throw socket_error("failed creating socket");

	set_sock_opts(opts_);
}

void netkit::socket::native::native_sync_listener::bind() {
	if (platform::bind(sockfd_, addr_.get_sa(), addr_.get_sa_len()) < 0) {
		throw socket_error("bind failed: " + platform::last_error_message());
	}

	bound_ = true;
}

void netkit::socket::native::native_sync_listener::unbind() {
	this->close();
}

void netkit::socket::native::native_sync_listener::bind(const addr& addr) {
	if (bound_) {
		throw socket_error{"bind failed"};
	}

	if (platform::bind(sockfd_, addr.get_sa(), addr.get_sa_len()) < 0) {
		throw socket_error("bind failed");
	}

	addr_ = addr;
	bound_ = true;
}

void netkit::socket::native::native_sync_listener::listen(int backlog) {
	if (!bound_) throw socket_error("listener not bound");

	if (platform::listen(this->sockfd_, backlog == -1 ? SOMAXCONN : backlog) < 0) {
		throw socket_error("failed to listen on socket");
	}

	listening_ = true;
}

void netkit::socket::native::native_sync_listener::listen() {
	this->listen(-1);
}

std::unique_ptr<netkit::socket::native::basic_native_sync_socket>
netkit::socket::native::native_sync_listener::accept() {
	sockaddr_storage client_addr{};
	socklen_t addr_len = sizeof(client_addr);

	auto client_sockfd = platform::accept(this->sockfd_, reinterpret_cast<sockaddr*>(&client_addr), &addr_len);
	if (!platform::valid_socket(client_sockfd)) {
		throw socket_error("failed to accept connection: " + std::string(std::strerror(errno)));
	}

#ifndef NETKIT_DKP
	if (this->type_ == type::uds) {
		return std::make_unique<native_sync_socket>(client_sockfd, socket::addr(reinterpret_cast<const sockaddr_un*>(&client_addr)->sun_path), this->type_);
	}
#endif

	auto peer = native::get_peer(client_sockfd);
	return std::make_unique<native_sync_socket>(client_sockfd, peer, this->type_);
}

void netkit::socket::native::native_sync_listener::close() noexcept {
	if (platform::valid_socket(sockfd_)) {
		platform::close_socket(sockfd_);
		this->bound_ = this->listening_ = false;
	}
}

netkit::socket::fd_t netkit::socket::native::native_sync_listener::native_handle() const{
	return sockfd_;
}

const netkit::socket::addr& netkit::socket::native::native_sync_listener::get_local_endpoint() const {
	return addr_;
}

