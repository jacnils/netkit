/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file basic_async_sock.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides a basic interface for asynchronous sockets.
 */
#pragma once

#include <netkit/definitions.hpp>

#ifdef NETKIT_LINUX

#include <memory>
#include <netkit/sock/addr.hpp>
#include <netkit/sock/addr_type.hpp>
#include <netkit/io/io_context.hpp>

namespace netkit::sock {
	/**
	* @brief A class that represents an asynchronous socket.
	* @note This class is an abstract base class and should not be instantiated directly.
	* @note Use the async_sock class instead.
	*/
    class NETKIT_API basic_async_sock {
    public:
        virtual ~basic_async_sock() = default;
        virtual netkit::io::task<void> connect() = 0;
		virtual void bind() = 0;
    	virtual void unbind() = 0;
    	virtual void listen() = 0;
    	virtual void listen(int backlog) = 0;
    	virtual netkit::io::task<std::unique_ptr<basic_async_sock>> accept() = 0;
    	virtual netkit::io::task<std::size_t> send(const void* buf, std::size_t len) = 0;
    	[[nodiscard]] virtual netkit::io::task<std::size_t> recv(void* buf, std::size_t len) = 0;
    	virtual void close() = 0;

    	/* ... */
    	virtual addr& get_addr() {
	        throw std::logic_error{"socket does not have an addr object"};
        }
        [[nodiscard]] virtual const addr& get_addr() const {
	        throw std::logic_error{"socket does not have an addr object"};
        }
        virtual void clear_overflow_bytes() const {}
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

#endif