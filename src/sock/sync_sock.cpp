/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file sync_sock.cpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Implementation of the synchronous socket class.
 */
#include <cstring>
#include <netkit/except.hpp>
#include <netkit/sock/sync_sock.hpp>
#include <netkit/sock/sock_peer.hpp>

#ifdef NETKIT_WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <afunix.h>
#include <cstring>
#elif NETKIT_UNIX
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#include <chrono>

const sockaddr* netkit::sock::sync_sock::get_sa() const {
    return reinterpret_cast<const sockaddr*>(&sa_storage);
}

socklen_t netkit::sock::sync_sock::get_sa_len() const {
    if (addr.is_ipv4()) return sizeof(sockaddr_in);
    if (addr.is_ipv6()) return sizeof(sockaddr_in6);
    if (addr.is_file_path()) {
        const auto& path = addr.get_path();
        return static_cast<socklen_t>(offsetof(sockaddr_un, sun_path) + path.string().size() + 1);
    }

    throw netkit::socket_error("invalid address type");
}

void netkit::sock::sync_sock::prep_sa() {
	memset(&sa_storage, 0, sizeof(sa_storage));

	if (addr.is_ipv4()) {
		auto* sa4 = reinterpret_cast<sockaddr_in*>(&sa_storage);
		sa4->sin_family = AF_INET;
		sa4->sin_port = htons(addr.get_port());
		if (inet_pton(AF_INET, addr.get_ip().c_str(), &sa4->sin_addr) <= 0) {
			throw parsing_error("invalid IPv4 address");
		}
	} else if (addr.is_ipv6()) {
		auto* sa6 = reinterpret_cast<sockaddr_in6*>(&sa_storage);
		sa6->sin6_family = AF_INET6;
		sa6->sin6_port = htons(addr.get_port());

		std::string ip = addr.get_ip();
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
	} else if (addr.is_file_path()) {
		auto* sa_un = reinterpret_cast<sockaddr_un*>(&sa_storage);
		sa_un->sun_family = AF_UNIX;
		const auto& path = addr.get_path().string();
		if (path.size() >= sizeof(sa_un->sun_path)) {
			throw socket_error("UNIX socket path too long");
		}
		std::memcpy(sa_un->sun_path, path.c_str(), path.size() + 1);
	} else {
		throw ip_error("invalid address type");
	}
}
#ifdef NETKIT_UNIX
void netkit::sock::sync_sock::set_sock_opts(sock_opt opts) const {
    if (opts & sock_opt::reuse_addr) {
        ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opts, sizeof(opts));
    } else if (opts & sock_opt::no_reuse_addr) {
        ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, nullptr, 0);
    }
    if (opts & sock_opt::no_delay) {
        ::setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &opts, sizeof(opts));
    }
    if (opts & sock_opt::keep_alive) {
        ::setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &opts, sizeof(opts));
    } else if (opts & sock_opt::no_keep_alive) {
        ::setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, nullptr, 0);
    }
    if (opts & sock_opt::no_blocking) {
        int flags = fcntl(this->sockfd, F_GETFL, 0);
        if (flags < 0) {
            ::close(this->sockfd);
            throw socket_error("failed to get socket flags");
        }
        if (fcntl(this->sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
            ::close(this->sockfd);
            throw socket_error("failed to set socket to non-blocking mode");
        }
    } else if (opts & sock_opt::blocking) {
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
void netkit::sock::sync_sock::set_sock_opts(sock_opt opts) const {
    if (opts & sock_opt::reuse_addr) {
        BOOL optval = TRUE;
        if (setsockopt(this->sockfd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&optval), sizeof(optval)) == SOCKET_ERROR) {
            closesocket(this->sockfd);
            throw socket_error("failed to set SO_REUSEADDR");
        }
    } else if (opts & sock_opt::no_reuse_addr) {
        BOOL optval = FALSE;
        if (setsockopt(this->sockfd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&optval), sizeof(optval)) == SOCKET_ERROR) {
            closesocket(this->sockfd);
            throw socket_error("failed to clear SO_REUSEADDR");
        }
    }
	if ((opts & sock_opt::no_delay) && type == sock_type::tcp) {
        BOOL optval = TRUE;
        if (setsockopt(this->sockfd, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&optval), sizeof(optval)) == SOCKET_ERROR) {
            closesocket(this->sockfd);
            throw socket_error("failed to set TCP_NODELAY");
        }
    }
    if (opts & sock_opt::keep_alive) {
        BOOL optval = TRUE;
        if (setsockopt(this->sockfd, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char*>(&optval), sizeof(optval)) == SOCKET_ERROR) {
            closesocket(this->sockfd);
            throw socket_error("failed to set SO_KEEPALIVE");
        }
    } else if (opts & sock_opt::no_keep_alive) {
        BOOL optval = FALSE;
        if (setsockopt(this->sockfd, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char*>(&optval), sizeof(optval)) == SOCKET_ERROR) {
            closesocket(this->sockfd);
            throw socket_error("failed to clear SO_KEEPALIVE");
        }
    }
    if (opts & sock_opt::no_blocking) {
        u_long mode = 1;
        if (ioctlsocket(this->sockfd, FIONBIO, &mode) == SOCKET_ERROR) {
            closesocket(this->sockfd);
            throw socket_error("failed to set socket to non-blocking mode");
        }
    } else if (opts & sock_opt::blocking) {
        u_long mode = 0;
        if (ioctlsocket(this->sockfd, FIONBIO, &mode) == SOCKET_ERROR) {
            closesocket(this->sockfd);
            throw socket_error("failed to set socket to blocking mode");
        }
    }
}
#endif

#ifdef NETKIT_UNIX
netkit::sock::sync_sock::sync_sock(const sock_addr& addr, sock_type t, sock_opt opts) : addr(addr), type(t) {
    this->sockfd = -1;

    if (addr.get_ip().empty() && !addr.is_file_path()) {
        throw socket_error("IP address/file path is empty");
    }

    if (t != sock_type::unix) {
        this->sockfd = ::socket(addr.is_ipv6() ? AF_INET6 : AF_INET,
                                                          t == sock_type::tcp ? SOCK_STREAM : SOCK_DGRAM, 0);
    } else {
        this->sockfd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    }

    if (this->sockfd < 0) {
        throw socket_error("failed to create socket");
    }

    if (this->sockfd >= 0) {
        this->set_sock_opts(opts);
    } else {
        throw socket_error("cannot set options on invalid socket");
    }

    this->prep_sa();
}

netkit::sock::sync_sock::sync_sock(int existing_fd, const sock_addr& peer, sock_type t, sock_opt opts)
    : addr(peer), type(t), sockfd(existing_fd) {
    if (sockfd < 0) throw socket_error("invalid fd");
    if (this->sockfd >= 0) {
        this->set_sock_opts(opts);
    } else {
        throw socket_error("cannot set options on invalid socket");
    }

    this->prep_sa();
}
#endif
#ifdef NETKIT_WINDOWS
netkit::sock::sync_sock::sync_sock(const sock_addr& addr, sock_type t, sock_opt opts)
    : addr(addr), type(t) {

    if (addr.get_ip().empty() && !addr.is_file_path()) {
        throw socket_error("IP address or file path is empty");
    }

    int domain = AF_UNIX;
    int sock_type = SOCK_STREAM;
    int protocol = 0;

    if (t != sock_type::unix) {
        domain = addr.is_ipv6() ? AF_INET6 : AF_INET;
        sock_type = (t == sock_type::tcp) ? SOCK_STREAM : SOCK_DGRAM;
        protocol = (t == sock_type::tcp) ? IPPROTO_TCP : IPPROTO_UDP;
    } else {
        protocol = 0;
    }

    this->sockfd = socket(domain, sock_type, protocol);
    if (this->sockfd == INVALID_SOCKET) {
        throw socket_error("Failed to create socket");
    }

    this->set_sock_opts(opts);
    this->prep_sa();
}
#endif
#ifdef NETKIT_UNIX
netkit::sock::sync_sock::~sync_sock() {
    if (this->sockfd == -1) {
        return;
    }
    if (::close(this->sockfd) < 0) {
        ;
    }
}
#endif
#ifdef NETKIT_WINDOWS
netkit::sock::sync_sock::~sync_sock() {
    if (this->sockfd == INVALID_SOCKET) {
        return;
    }

    if (::closesocket(this->sockfd) == SOCKET_ERROR) {
        return;
    }

    this->sockfd = INVALID_SOCKET;
}
#endif

netkit::sock::sock_addr& netkit::sock::sync_sock::get_addr() {
    return this->addr;
}

const netkit::sock::sock_addr& netkit::sock::sync_sock::get_addr() const {
    return this->addr;
}
#ifdef NETKIT_UNIX
void netkit::sock::sync_sock::connect() {
    if (::connect(this->sockfd, this->get_sa(), this->get_sa_len()) < 0) {
        throw netkit::socket_error("failed to connect to server");
    }
}
#endif
#ifdef NETKIT_WINDOWS
void netkit::sock::sync_sock::connect() {
    if (::connect(this->sockfd, this->get_sa(), this->get_sa_len()) == SOCKET_ERROR) {
        throw socket_error("failed to connect to server");
    }
}
#endif
#ifdef NETKIT_UNIX
void netkit::sock::sync_sock::bind() {
    this->bound = true;

    auto ret = ::bind(this->sockfd, this->get_sa(), this->get_sa_len());

    if (ret < 0) {
        throw socket_error("failed to bind socket: " + std::to_string(ret));
    }
}
#endif
#ifdef NETKIT_WINDOWS
void netkit::sock::sync_sock::bind() {
    this->bound = true;

    int result = ::bind(this->sockfd, this->get_sa(), this->get_sa_len());

    if (result == SOCKET_ERROR) {
        int err = WSAGetLastError();
        throw socket_error("failed to bind socket, error code: " + std::to_string(err));
    }
}
#endif
#ifdef NETKIT_UNIX
void netkit::sock::sync_sock::unbind() {
    if (this->bound) {
        if (::close(this->sockfd) < 0) {
            throw socket_error("failed to unbind socket");
        }
        this->bound = false;
    }
}
#endif
#ifdef NETKIT_WINDOWS
void netkit::sock::sync_sock::unbind() {
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
void netkit::sock::sync_sock::listen(int backlog) {
    if (::listen(this->sockfd, backlog == -1 ? SOMAXCONN : backlog) < 0) {
        throw socket_error("failed to listen on socket");
    }
}
#endif
#ifdef NETKIT_WINDOWS
void netkit::sock::sync_sock::listen(int backlog) {
    if (::listen(this->sockfd, backlog == -1 ? SOMAXCONN : backlog) == SOCKET_ERROR) {
        int err = WSAGetLastError();
        throw socket_error("failed to listen socket, error code: " + std::to_string(err));
    }
}
#endif
void netkit::sock::sync_sock::listen() {
    listen(-1);
}
#ifdef NETKIT_UNIX
std::unique_ptr<netkit::sock::basic_sync_sock> netkit::sock::sync_sock::accept() {
    sockaddr_storage client_addr{};
    socklen_t addr_len = sizeof(client_addr);

    int client_sockfd = ::accept(this->sockfd, reinterpret_cast<sockaddr*>(&client_addr), &addr_len);
    if (client_sockfd < 0) {
        throw socket_error("failed to accept connection: " + std::string(strerror(errno)));
    }

    auto peer = sock::get_peer(client_sockfd);

    return std::make_unique<sync_sock>(client_sockfd, peer, this->type);
}
#endif
#ifdef NETKIT_WINDOWS
std::unique_ptr<netkit::sock::basic_sync_sock> netkit::sock::sync_sock::accept() {
    sockaddr_storage client_addr{};
    int addr_len = sizeof(client_addr);

    SOCKET client_sockfd = ::accept(this->sockfd, reinterpret_cast<sockaddr*>(&client_addr), &addr_len);
    if (client_sockfd == INVALID_SOCKET) {
        int err = WSAGetLastError();
        throw socket_error("failed to accept connection, error code: " + std::to_string(err));
    }

    auto peer = sock::get_peer(client_sockfd);
    auto handle = std::make_unique<sync_sock>(peer, this->type);
    handle->sockfd = client_sockfd;

    return handle;
}
#endif
#ifdef NETKIT_UNIX
int netkit::sock::sync_sock::send(const void* buf, size_t len) {
    size_t total_sent = 0;
    const char* data = static_cast<const char*>(buf);

    while (total_sent < len) {
        ssize_t sent = ::send(this->sockfd, data + total_sent, len - total_sent, 0);
        if (sent <= 0) {
            return static_cast<int>(sent);
        }
        total_sent += sent;
    }

    return static_cast<int>(total_sent);
}
#endif
#ifdef NETKIT_WINDOWS
int netkit::sock::sync_sock::send(const void* buf, size_t len) {
    size_t total_sent = 0;
    const char* data = static_cast<const char*>(buf);

    while (total_sent < len) {
        int ret = ::send(this->sockfd, data + total_sent, static_cast<int>(len - total_sent), 0);
        if (ret == SOCKET_ERROR) {
            int err = WSAGetLastError();
            throw socket_error("send() failed, error code: " + std::to_string(err));
        }
        if (ret == 0) {
            break;
        }
        total_sent += ret;
    }

    return static_cast<int>(total_sent);
}
#endif
void netkit::sock::sync_sock::send(const std::string& buf) {
    static_cast<void>(this->send(buf.c_str(), buf.length()));
}

std::string netkit::sock::sync_sock::overflow_bytes() const {
    return old_bytes;
}

void netkit::sock::sync_sock::clear_overflow_bytes() const {
    old_bytes.clear();
}
#ifdef NETKIT_UNIX
netkit::sock::sock_recv_result netkit::sock::sync_sock::recv(const int timeout_seconds, const std::string& match, size_t eof) {
    std::string data = old_bytes;
    old_bytes.clear();

    if (eof != 0 && data.size() >= eof) {
        if (data.size() > eof) {
            old_bytes = data.substr(eof);
            data.resize(eof);
        }
        return {data, sock_recv_status::success};
    }

    if (!match.empty()) {
        size_t pos = data.find(match);
        if (pos != std::string::npos) {
            old_bytes = data.substr(pos + match.size());
            data.resize(pos + match.size());
            return {data, sock_recv_status::success};
        }
    }

    auto start = std::chrono::steady_clock::now();

    while (true) {
        auto elapsed = std::chrono::steady_clock::now() - start;
        auto remaining = std::chrono::seconds(timeout_seconds) - elapsed;
        if (remaining <= std::chrono::seconds(0) && timeout_seconds != -1) {
            return {data, sock_recv_status::timeout};
        }

        timeval tv{};
        tv.tv_sec = std::chrono::duration_cast<std::chrono::seconds>(remaining).count();
        tv.tv_usec = 0;

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(this->sockfd, &readfds);

        if (this->sockfd < 0) throw socket_error("invalid socket descriptor");
        int ret = ::select(this->sockfd + 1, &readfds, nullptr, nullptr,
                                               timeout_seconds == -1 ? nullptr : &tv);
        if (ret < 0) throw socket_error("select() failed");
        if (ret == 0) return {data, sock_recv_status::timeout};

        if (FD_ISSET(this->sockfd, &readfds)) {
            size_t bytes_to_read = 8192;
            if (eof != 0 && data.size() + bytes_to_read > eof) {
                bytes_to_read = eof - data.size();
            }

            char buf[8192];
            ssize_t received = ::recv(this->sockfd, buf, bytes_to_read, 0);
            if (received < 0) {
                if (errno == EINTR) continue;
                throw socket_error("recv() failed");
            }
            if (received == 0) return {data, sock_recv_status::closed};

            data.append(buf, static_cast<std::size_t>(received));

            if (eof != 0 && data.length() > eof) {
                old_bytes = data.substr(eof);
                data.resize(eof);
            }
            if (eof != 0 && data.length() >= eof) {
                return {data, sock_recv_status::success};
            }

            if (!match.empty()) {
                size_t pos = data.find(match);
                if (pos != std::string::npos) {
                    old_bytes = data.substr(pos + match.size());
                    data.resize(pos + match.size());
                    return {data, sock_recv_status::success};
                }
            }
        }
    }
}

netkit::sock::sock_recv_result netkit::sock::sync_sock::primitive_recv() {
    for (;;) {
        char buf[8192];
        ssize_t n = ::recv(this->sockfd, buf, sizeof(buf), 0);
        if (n > 0)
            return {{buf, buf + n}, sock_recv_status::success};
        if (n == 0)
            return {{}, sock_recv_status::closed};
        if (errno == EINTR)
            continue;
        throw netkit::socket_error("recv failed");
    }
}
#endif
#ifdef NETKIT_WINDOWS
netkit::sock::sock_recv_result netkit::sock::sync_sock::recv(const int timeout_seconds, const std::string& match, size_t eof) {
    std::string data = old_bytes;
    old_bytes.clear();

    if (eof != 0 && data.size() >= eof) {
        if (data.size() > eof) {
            old_bytes = data.substr(eof);
            data.resize(eof);
        }
        return {data, sock_recv_status::success};
    }

    if (!match.empty()) {
        size_t pos = data.find(match);
        if (pos != std::string::npos) {
            old_bytes = data.substr(pos + match.size());
            data.resize(pos + match.size());
            return {data, sock_recv_status::success};
        }
    }

    auto start = std::chrono::steady_clock::now();

    while (true) {
        auto elapsed = std::chrono::steady_clock::now() - start;
        auto remaining = std::chrono::seconds(timeout_seconds) - elapsed;
        if (timeout_seconds == -1) {
            remaining = std::chrono::hours(24 * 365 * 100);
        }
        if (remaining <= std::chrono::seconds(0) && timeout_seconds != -1) {
            return {data, sock_recv_status::timeout};
        }

        timeval tv{};
        tv.tv_sec = static_cast<long>(std::chrono::duration_cast<std::chrono::seconds>(remaining).count());
        tv.tv_usec = 0;

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(this->sockfd, &readfds);

        if (this->sockfd < 0) throw socket_error("invalid socket descriptor");

        int ret = ::select(0, &readfds, nullptr, nullptr, timeout_seconds == -1 ? nullptr : &tv);
        if (ret == SOCKET_ERROR) {
            throw socket_error("select() failed");
        }
        if (ret == 0) {
            return {data, sock_recv_status::timeout};
        }

        if (FD_ISSET(this->sockfd, &readfds)) {
            size_t bytes_to_read = 8192;
            if (eof != 0 && data.size() + bytes_to_read > eof) {
                bytes_to_read = eof - data.size();
            }

            char buf[8192];
            int received = ::recv(this->sockfd, buf, static_cast<int>(bytes_to_read), 0);
            if (received == SOCKET_ERROR) {
                int err = WSAGetLastError();
                if (err == WSAEINTR) continue;
                throw socket_error("recv() failed");
            }
            if (received == 0) {
                return {data, sock_recv_status::closed};
            }

            data.append(buf, static_cast<std::size_t>(received));

            if (eof != 0 && data.length() > eof) {
                old_bytes = data.substr(eof);
                data.resize(eof);
            }
            if (eof != 0 && data.length() >= eof) {
                return {data, sock_recv_status::success};
            }

            if (!match.empty()) {
                size_t pos = data.find(match);
                if (pos != std::string::npos) {
                    old_bytes = data.substr(pos + match.size());
                    data.resize(pos + match.size());
                    return {data, sock_recv_status::success};
                }
            }
        }
    }
}

netkit::sock::sock_recv_result netkit::sock::sync_sock::primitive_recv() {
    constexpr size_t buffer_size = 8192;
    char buf[buffer_size];

    for (;;) {
        int n = ::recv(this->sockfd, buf, static_cast<int>(buffer_size), 0);
        if (n > 0) {
            return {std::string(buf, buf + n), sock_recv_status::success};
        } else if (n == 0) {
            return {{}, sock_recv_status::closed};
        } else {
            int err = WSAGetLastError();
            if (err == WSAEINTR || err == WSAEWOULDBLOCK || err == WSAEINPROGRESS) {
                continue;
            }
            throw std::runtime_error("recv failed: WSA error " + std::to_string(err));
        }
    }
}
#endif

netkit::sock::sock_recv_result netkit::sock::sync_sock::recv(const int timeout_seconds) {
    return recv(timeout_seconds, "", 0);
}

netkit::sock::sock_recv_result netkit::sock::sync_sock::recv(const int timeout_seconds, const std::string& match) {
    return this->recv(timeout_seconds, match, 0);
}

netkit::sock::sock_recv_result netkit::sock::sync_sock::recv(const int timeout_seconds, size_t eof) {
    return this->recv(timeout_seconds, "", eof);
}

#ifdef NETKIT_UNIX
void netkit::sock::sync_sock::close() {
    if (this->sockfd == -1) {
        return;
    }

    (void)::close(this->sockfd);
    this->sockfd = -1;
}
#endif
#ifdef NETKIT_WINDOWS
void netkit::sock::sync_sock::close() {
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
[[nodiscard]] netkit::sock::sock_addr netkit::sock::sync_sock::get_peer() const {
    return netkit::sock::get_peer(this->sockfd);
}