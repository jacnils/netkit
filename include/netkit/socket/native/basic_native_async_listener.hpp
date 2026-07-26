#pragma once

#include <netkit/definitions.hpp>

#ifdef NETKIT_LINUX

#include <memory>
#include <netkit/io/io_context.hpp>
#include <netkit/socket/addr.hpp>
#include <netkit/socket/addr_type.hpp>
#include <netkit/socket/native/basic_native_async_sock.hpp>

namespace netkit::sock::native {
    class NETKIT_API basic_native_async_listener {
    public:
        virtual ~basic_native_async_listener() = default;
		virtual void bind() = 0;
    	virtual void unbind() = 0;
    	virtual void listen() = 0;
    	virtual void listen(int backlog) = 0;
    	virtual netkit::io::task<std::unique_ptr<basic_native_async_sock>> accept() = 0;
    	virtual void close() noexcept = 0;

    	[[nodiscard]] virtual const addr& get_local_endpoint() const {
	        throw std::logic_error{"socket does not have an addr object"};
        }
    	[[nodiscard]] virtual fd_t native_handle() const {
    		throw std::logic_error{"socket does not have a native handle"};
    	}
    };
}

#endif