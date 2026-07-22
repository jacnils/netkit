/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file basic_sync_sock.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides a basic interface for synchronous sockets.
 */
#pragma once

#include <memory>
#include <netkit/sock/addr.hpp>
#include <netkit/sock/addr_type.hpp>

namespace netkit::sock {
	/**
	* @brief A class that represents a synchronous socket.
	* @note This class is an abstract base class and should not be instantiated directly.
	* @note Use the sync_sock class instead.
	*/
    class NETKIT_API basic_sync_sock {
    public:
        virtual ~basic_sync_sock() = default;
        virtual void connect() = 0;
        virtual void bind() = 0;
        virtual void unbind() = 0;
        virtual void listen(int backlog) = 0;
        virtual void listen() = 0;
        [[nodiscard]] virtual std::unique_ptr<basic_sync_sock> accept() = 0;
        virtual int send(const void* buf, size_t len) = 0;
        virtual void send(const std::string& buf) = 0;
        [[nodiscard]] virtual recv_result recv(int timeout_seconds) = 0;
        [[nodiscard]] virtual recv_result recv(int timeout_seconds, const std::string& match) = 0;
        [[nodiscard]] virtual recv_result recv(int timeout_seconds, const std::string& match, size_t eof) = 0;
        [[nodiscard]] virtual recv_result recv(int timeout_seconds, size_t eof) = 0;
        [[nodiscard]] virtual recv_result recv() = 0;
        [[nodiscard]] virtual std::string overflow_bytes() const { return {}; };
        virtual addr& get_addr() {
	        throw std::logic_error{"socket does not have an addr object"};
        }
        [[nodiscard]] virtual const addr& get_addr() const {
	        throw std::logic_error{"socket does not have an addr object"};
        }
        virtual void clear_overflow_bytes() const {}
        virtual void close() = 0;
        [[nodiscard]] virtual addr get_peer() const {
	        throw std::logic_error{"socket does not have a peer"};
        };
    	[[nodiscard]] virtual fd_t native_handle() const {
    		throw std::logic_error{"socket does not have a native handle"};
    	}
    	virtual void set_sock_opts(opt opts) {
    		throw std::logic_error{"socket does not have opts to set"};
    	}
    };
}