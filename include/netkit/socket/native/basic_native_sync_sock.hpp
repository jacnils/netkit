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
#include <netkit/socket/addr.hpp>
#include <netkit/socket/addr_type.hpp>

namespace netkit::sock::native {
	/**
	* @brief A class that represents a synchronous socket.
	* @note This class is an abstract base class and should not be instantiated directly.
	* @note Use the sync_sock class instead.
	*/
    class NETKIT_API basic_native_sync_sock {
    public:
        virtual ~basic_native_sync_sock() = default;
    	virtual void connect() = 0;
        virtual std::size_t send(const void* buf, std::size_t len) = 0;
        [[nodiscard]] virtual std::size_t recv(void* buf, std::size_t len) = 0;
        virtual addr& get_addr() {
	        throw std::logic_error{"socket does not have an addr object"};
        }
        [[nodiscard]] virtual const addr& get_addr() const {
	        throw std::logic_error{"socket does not have an addr object"};
        }
        virtual void close() noexcept = 0;
    	[[nodiscard]] virtual fd_t native_handle() const {
    		throw std::logic_error{"socket does not have a native handle"};
    	}
    	virtual void set_sock_opts(opt opts) {
    		throw std::logic_error{"socket does not have opts to set"};
    	}
    	[[nodiscard]] virtual addr get_peer() const = 0;

    };
}