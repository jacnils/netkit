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

#include <memory>
#include <netkit/io/io_context.hpp>
#include <netkit/socket/addr.hpp>
#include <netkit/socket/addr_type.hpp>

namespace netkit::sock::native {
	/**
	* @brief A class that represents an asynchronous socket.
	* @note This class is an abstract base class and should not be instantiated directly.
	* @note Use the async_sock class instead.
	*/
    class NETKIT_API basic_native_async_sock {
    public:
        virtual ~basic_native_async_sock() = default;
        virtual netkit::io::task<void> connect() = 0;
    	virtual netkit::io::task<std::size_t> send(const void* buf, std::size_t len) = 0;
    	[[nodiscard]] virtual netkit::io::task<std::size_t> recv(void* buf, std::size_t len) = 0;
    	virtual void close() noexcept = 0;

    	virtual addr& get_addr() {
	        throw std::logic_error{"socket does not have an addr object"};
        }
        [[nodiscard]] virtual const addr& get_addr() const {
	        throw std::logic_error{"socket does not have an addr object"};
        }
        [[nodiscard]] virtual addr get_peer() const {
	        throw std::logic_error{"socket does not have a peer"};
        };
    	virtual void set_sock_opts(opt opts) {
    		throw std::logic_error{"socket does not have opts to set"};
    	}
    	[[nodiscard]] virtual fd_t native_handle() const {
    		throw std::logic_error{"socket does not have a native handle"};
    	}
    };
}