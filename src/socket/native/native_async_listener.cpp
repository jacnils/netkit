#include <netkit/platform/socket.hpp>

#include <cstring>
#include <netkit/except.hpp>
#include <netkit/socket/native/native_async_listener.hpp>
#include <netkit/socket/native/native_async_sock.hpp>
#include <netkit/socket/native/peer_helper.hpp>

#ifdef NETKIT_WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <afunix.h>
#elif defined(NETKIT_UNIX) && !defined(NETKIT_DKP)
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#elif defined(NETKIT_DKP)
#include <network.h>
#endif

void netkit::sock::native::native_async_listener::set_sock_opts(opt opts) const {
	platform::set_sock_opts(this->sockfd_, opts);
}

netkit::sock::native::native_async_listener::native_async_listener(io::io_context& ctx, const addr& address, type t, opt opts) : context_(ctx), addr_(address), type_(t), opts_(opts) {
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

netkit::sock::native::native_async_listener::~native_async_listener() {
	this->native_async_listener::close();
}

void netkit::sock::native::native_async_listener::bind() {
	if (platform::bind(sockfd_, addr_.get_sa(), addr_.get_sa_len()) < 0) {
		throw socket_error("bind failed");
	}

	bound_ = true;
}

void netkit::sock::native::native_async_listener::bind(const addr& addr) {
	if (bound_) {
		throw socket_error{"bind failed"};
	}

	if (platform::bind(sockfd_, addr.get_sa(), addr.get_sa_len()) < 0) {
		throw socket_error("bind failed");
	}

	addr_ = addr;
	bound_ = true;
}

void netkit::sock::native::native_async_listener::unbind() {
	this->close();
}

void netkit::sock::native::native_async_listener::listen(int backlog) {
	if (!bound_) throw socket_error("listener not bound");

	if (platform::listen(this->sockfd_, backlog == -1 ? SOMAXCONN : backlog) < 0) {
		throw socket_error("failed to listen on socket");
	}

	listening_ = true;
}

void netkit::sock::native::native_async_listener::listen() {
	this->listen(-1);
}

netkit::io::task<std::unique_ptr<netkit::sock::native::basic_native_async_sock>>
netkit::sock::native::native_async_listener::accept() {
	while (true) {
		sockaddr_storage client_addr{};
		socklen_t addr_len = sizeof(client_addr);

		fd_t client_sockfd = platform::accept(
			this->sockfd_,
			reinterpret_cast<sockaddr*>(&client_addr),
			&addr_len
		);

		if (platform::valid_socket(client_sockfd)) {
#ifndef NETKIT_DKP
			if (this->type_ == type::uds) {
				co_return std::make_unique<native_async_sock>(
					this->context_,
					client_sockfd,
					sock::addr(
						reinterpret_cast<const sockaddr_un*>(&client_addr)->sun_path
					),
					this->type_
				);
			}
#endif

			auto peer = sock::native::get_peer(client_sockfd);

			co_return std::make_unique<native_async_sock>(
				this->context_,
				client_sockfd,
				peer,
				this->type_
			);
		}

		if (platform::last_socket_error() == platform::socket_err::would_block) {
			co_await this->context_.wait_readable(this->sockfd_);
			continue;
		}

		throw socket_error("failed to accept connection: " + platform::last_error_message());
	}
}

void netkit::sock::native::native_async_listener::close() noexcept {
	if (platform::valid_socket(sockfd_)) {
		platform::close_socket(sockfd_);
		this->bound_ = this->listening_ = false;
	}
}

netkit::sock::fd_t netkit::sock::native::native_async_listener::native_handle() const {
	return sockfd_;
}

const netkit::sock::addr& netkit::sock::native::native_async_listener::get_local_endpoint() const {
	return addr_;
}

