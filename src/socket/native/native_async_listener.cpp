#include <netkit/platform/socket.hpp>

#include <cstring>
#include <netkit/except.hpp>
#include <netkit/socket/native/native_async_listener.hpp>
#include <netkit/socket/native/native_async_sock.hpp>
#include <netkit/socket/native/peer_helper.hpp>
#include <unistd.h>

#ifdef NETKIT_WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <afunix.h>
#elif NETKIT_UNIX
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#endif

void netkit::sock::native::native_async_listener::set_sock_opts(opt opts) const {
	platform::set_sock_opts(this->sockfd_, opts);
}

const sockaddr* netkit::sock::native::native_async_listener::get_sa() const {
	return reinterpret_cast<const sockaddr*>(&sa_storage_);
}

socklen_t netkit::sock::native::native_async_listener::get_sa_len() const {
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

void netkit::sock::native::native_async_listener::prep_sa() {
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

netkit::sock::native::native_async_listener::native_async_listener(io::io_context& ctx, const addr& address, type t, opt opts) : context_(ctx), addr_(address), type_(t), opts_(opts) {
	sockfd_ = platform::socket(addr_.is_ipv6() ? AF_INET6 : AF_INET, SOCK_STREAM, 0);

	if (!platform::valid_socket(sockfd_))
		throw socket_error("failed creating socket");

	set_sock_opts(opts_);
	prep_sa();
}

netkit::sock::native::native_async_listener::~native_async_listener() {
	this->native_async_listener::close();
}

void netkit::sock::native::native_async_listener::bind() {
	if (platform::bind(sockfd_, get_sa(), get_sa_len()) < 0) {
		throw socket_error("bind failed");
	}

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

