#include <netkit/stream/socket_stream.hpp>
#include <netkit/definitions.hpp>
#include <netkit/except.hpp>

#ifdef NETKIT_UNIX
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#include <cstring>
#include <netkit/socket/native/peer_helper.hpp>
#include <netkit/socket/addr.hpp>
#include <netkit/socket/addr_type.hpp>
#include <netkit/definitions.hpp>

netkit::sock::addr netkit::sock::native::get_peer(fd_t sockfd) {
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
		port,
		netkit::sock::addr_type::ipv4
	};
#else
	sockaddr_storage addr_storage{};
	socklen_t addr_len = sizeof(addr_storage);

	if (getpeername(sockfd, reinterpret_cast<sockaddr*>(&addr_storage), &addr_len) < 0) {
		throw netkit::socket_error("getpeername() failed: " + std::string(std::strerror(errno)));
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

	return netkit::sock::addr{
		ip_str,
		port,
		(addr_storage.ss_family == AF_INET) ? sock::addr_type::ipv4 : sock::addr_type::ipv6
	};
#endif
}