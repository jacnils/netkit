/* There would normally be a useful file header here, but I am going to use the header
 * to complain, instead:
 *
 * This file only has to exist because Microsoft Windows is the most retarded operating system
 * that has ever existed. In (somewhat) recent years, they've implemented tons of features that were essentially
 * backports from the Unix socket implementation, such as support for
 * UDS sockets (in typical Microsoft fashion, it's half-assed.)
 *
 * Instead of doing this properly, i.e. copying it in such a way that code written for Unix systems
 * just works, in true Microsoft fashion, they ALWAYS have to fuck it up their own shitty Windows-isms,
 * like defining a special unsigned SOCKET type. Oh, we're so special, we have to do it in our own shitty
 * fucking way, so that consumers writing cross-platform code have to pollute their code with #ifdef cancer.
 *
 * On Unix systems, we have the concept of a file descriptor, which we store using a signed integer.
 * This makes checking the validity of a file descriptor trivial; just check whether it is greater than or equal to 0.
 *
 * On Windows however, we don't have file descriptors, but we still have socket identifiers, and instead of being
 * signed, they're unsigned, so we have to check if it's equal to INVALID_SOCKET which, further, means that
 * in order to support both operating systems, we have to have a bunch of shitty helper functions.
 *
 * That's what this header is for -- abstracting away a few of Microsoft's many horrible design choices.
 * Frankly, having to support Windows is a nuisance more than anything.
 *
 * Linux/BSD/macOS users, count your blessings; at least you don't have to deal with this utter piece of shit.
 */
#pragma once

#include <netkit/stream/socket_stream.hpp>

#include <netkit/definitions.hpp>
#include <netkit/except.hpp>
#include <netkit/socket/addr_type.hpp>

#ifdef NETKIT_WINDOWS
#include <winsock2.h>
#else
#include <netinet/tcp.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#endif

#ifdef NETKIT_DKP
#include <unordered_map>
#endif

namespace netkit::platform {

#ifdef NETKIT_WINDOWS
typedef SOCKET socket_t;
typedef int socket_result;
typedef int socket_length_t;

inline constexpr socket_t invalid_socket = INVALID_SOCKET;
#else
typedef int socket_t;
typedef ssize_t socket_result;
typedef socklen_t socket_length_t;

inline constexpr socket_t invalid_socket = -1;
#endif

enum class socket_err {
	none,
	would_block,
	in_progress,
	interrupted,
	connection_refused,
	timed_out,
	not_connected,
	unknown
};

inline bool valid_socket(socket_t s) noexcept {
#ifdef NETKIT_WINDOWS
	return s != invalid_socket;
#else
	return s >= 0;
#endif
}

inline void close_socket(socket_t s) noexcept {
#ifdef NETKIT_WINDOWS
	closesocket(s);
#else
	close(s);
#endif
}

inline void set_sock_opts(socket_t sockfd, socket::opt opts) {
#ifdef NETKIT_UNIX
	if (opts & netkit::socket::opt::reuse_addr) {
		::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opts, sizeof(opts));
	} else if (opts & netkit::socket::opt::no_reuse_addr) {
		::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, nullptr, 0);
	}
	if (opts & netkit::socket::opt::no_delay) {
		::setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &opts, sizeof(opts));
	}
	if (opts & netkit::socket::opt::keep_alive) {
		::setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &opts, sizeof(opts));
	} else if (opts & netkit::socket::opt::no_keep_alive) {
		::setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, nullptr, 0);
	}
	if (opts & netkit::socket::opt::no_blocking) {
		int flags = fcntl(sockfd, F_GETFL, 0);
		if (flags < 0) {
			::close(sockfd);
			throw socket_error("failed to get socket flags");
		}
		if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
			::close(sockfd);
			throw socket_error("failed to set socket to non-blocking mode");
		}
	} else if (opts & netkit::socket::opt::blocking) {
		int flags = fcntl(sockfd, F_GETFL, 0);
		if (flags < 0) {
			::close(sockfd);
			throw socket_error("failed to get socket flags");
		}
		if (fcntl(sockfd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
			::close(sockfd);
			throw socket_error("failed to set socket to blocking mode");
		}
	}
#elifdef NETKIT_WINDOWS
	if (opts & netkit::sock::opt::reuse_addr) {
		BOOL optval = TRUE;
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&optval), sizeof(optval)) == SOCKET_ERROR) {
			closesocket(sockfd);
			throw socket_error("failed to set SO_REUSEADDR");
		}
	} else if (opts & netkit::sock::opt::no_reuse_addr) {
		BOOL optval = FALSE;
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&optval), sizeof(optval)) == SOCKET_ERROR) {
			closesocket(sockfd);
			throw socket_error("failed to clear SO_REUSEADDR");
		}
	}
	if ((opts & netkit::sock::opt::no_delay)) {
		BOOL optval = TRUE;
		if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&optval), sizeof(optval)) == SOCKET_ERROR) {
			closesocket(sockfd);
			throw socket_error("failed to set TCP_NODELAY");
		}
	}
	if (opts & netkit::sock::opt::keep_alive) {
		BOOL optval = TRUE;
		if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char*>(&optval), sizeof(optval)) == SOCKET_ERROR) {
			closesocket(sockfd);
			throw socket_error("failed to set SO_KEEPALIVE");
		}
	} else if (opts & netkit::sock::opt::no_keep_alive) {
		BOOL optval = FALSE;
		if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char*>(&optval), sizeof(optval)) == SOCKET_ERROR) {
			closesocket(sockfd);
			throw socket_error("failed to clear SO_KEEPALIVE");
		}
	}
	if (opts & netkit::sock::opt::no_blocking) {
		u_long mode = 1;
		if (ioctlsocket(sockfd, FIONBIO, &mode) == SOCKET_ERROR) {
			closesocket(sockfd);
			throw socket_error("failed to set socket to non-blocking mode");
		}
	} else if (opts & netkit::sock::opt::blocking) {
		u_long mode = 0;
		if (ioctlsocket(sockfd, FIONBIO, &mode) == SOCKET_ERROR) {
			closesocket(sockfd);
			throw socket_error("failed to set socket to blocking mode");
		}
	}
#endif
}

inline socket_err last_socket_error() {
#ifdef NETKIT_WINDOWS

	switch (WSAGetLastError()) {
	case 0: return socket_err::none;
	case WSAEWOULDBLOCK: return socket_err::would_block;
	case WSAEINPROGRESS: return socket_err::in_progress;
	case WSAEINTR: return socket_err::interrupted;
	case WSAECONNREFUSED: return socket_err::connection_refused;
	case WSAETIMEDOUT: return socket_err::timed_out;
	case WSAENOTCONN: return socket_err::not_connected;
	default: return socket_err::unknown;
	}

#else
	switch (errno) {
	case 0: return socket_err::none;
#if EWOULDBLOCK == EAGAIN
	case EAGAIN:
		return socket_err::would_block;
#else
	case EAGAIN:
	case EWOULDBLOCK:
		return socket_err::would_block;
#endif
	case EINPROGRESS: return socket_err::in_progress;
	case EINTR: return socket_err::interrupted;
	case ECONNREFUSED: return socket_err::connection_refused;
	case ETIMEDOUT: return socket_err::timed_out;
	case ENOTCONN: return socket_err::not_connected;
	default: return socket_err::unknown;
	}
#endif
}

#ifdef NETKIT_WINDOWS
inline std::string last_error_message() {
	int err = WSAGetLastError();
	char* msg = nullptr;

	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		nullptr,
		err,
		0,
		(LPSTR)&msg,
		0,
		nullptr
	);

	std::string result = msg ? msg : "unknown";
	if (msg) LocalFree(msg);
	return result;
}
#else
inline std::string last_error_message() {
	return std::strerror(errno);
}
#endif

inline socket_t socket(int domain, int type, int protocol) {
	return ::socket(domain, type, protocol);
}

inline socket_result send(socket_t sock,const void* buffer,size_t length,int flags) {
#ifdef NETKIT_WINDOWS
	if (length > INT_MAX)
		length = INT_MAX;
#endif

	return ::send(
		sock,
#ifdef NETKIT_WINDOWS
		static_cast<const char*>(buffer),
		static_cast<int>(length),
#else
		buffer,
		length,
#endif
		flags
	);
}

inline socket_result recv(socket_t sock, void* buffer, std::size_t length, int flags = 0) {
#ifdef NETKIT_WINDOWS
	return ::recv(sock, static_cast<char*>(buffer), static_cast<int>(length), flags);
#else
	return ::recv(sock, buffer, length, flags);
#endif
}

// some nasty devkitpro hacks incoming
#ifdef NETKIT_DKP
inline std::unordered_map<socket_t, sock::addr>& peer_cache() {
	static std::unordered_map<socket_t, sock::addr> cache;
	return cache;
}

inline void cache_peer(socket_t fd, sock::addr peer) {
	peer_cache().emplace(fd, std::move(peer));
}

inline std::optional<sock::addr> take_cached_peer(socket_t fd) {
	auto& cache = peer_cache();

	auto it = cache.find(fd);

	if (it == cache.end())
		return std::nullopt;

	auto peer = std::move(it->second);
	cache.erase(it);

	return peer;
}
#endif

inline socket_t accept(socket_t sock, sockaddr* addr, socket_length_t* length) {
#ifndef NETKIT_DKP
	return ::accept(sock, addr, length);
#else
	socket_t client = ::accept(sock, addr, length);

	if (!platform::valid_socket(client))
		return client;

	auto* addr_in = reinterpret_cast<sockaddr_in*>(addr);

	char ip_str[INET_ADDRSTRLEN]{};

	inet_ntop(
		AF_INET,
		&addr_in->sin_addr,
		ip_str,
		sizeof(ip_str)
	);

	auto peer = sock::addr(
		ip_str,
		ntohs(addr_in->sin_port),
		sock::addr_type::ipv4
	);

	platform::cache_peer(
		client,
		std::move(peer)
	);

	return client;

#endif
}

inline int connect(socket_t sock, const sockaddr* addr, socket_length_t length) {
	return ::connect(sock, addr, length);
}

inline int bind(socket_t sock, const sockaddr* addr, socket_length_t length) {
	return ::bind(sock, addr, length);
}

inline int listen(socket_t sock, int backlog) {
	return ::listen(sock, backlog);
}

inline socket_result recvfrom(socket_t sock, void* buffer, size_t length, int flags, sockaddr* addr, socklen_t* addrlen) {
#ifdef NETKIT_WINDOWS
	return ::recvfrom(sock, static_cast<char*>(buffer), static_cast<int>(length), flags, addr, addrlen);
#else
	return ::recvfrom(sock, buffer, length, flags, addr, addrlen);
#endif
}

inline socket_result sendto(socket_t sock, const void* buffer, size_t length, int flags, const sockaddr* addr, socklen_t addrlen) {
#ifdef NETKIT_WINDOWS
	return ::sendto(sock, static_cast<const char*>(buffer), static_cast<int>(length), flags, addr, addrlen);
#else
	return ::sendto(sock, buffer, length, flags, addr, addrlen);
#endif
}

}
