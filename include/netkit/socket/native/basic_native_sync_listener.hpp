#pragma once

#include <netkit/socket/native/basic_native_sync_socket.hpp>

#include <memory>
#include <netkit/socket/addr.hpp>
#include <netkit/socket/addr_type.hpp>

namespace netkit::socket::native {
    class NETKIT_API basic_native_sync_listener {
    public:
        virtual ~basic_native_sync_listener() = default;

        virtual void bind() = 0;
    	virtual void bind(const addr& addr) = 0;
        virtual void unbind() = 0;

        virtual void listen(int backlog) = 0;
        virtual void listen() = 0;

        [[nodiscard]] virtual std::unique_ptr<basic_native_sync_socket> accept() = 0;

        [[nodiscard]] virtual const addr& get_local_endpoint() const {
	        throw std::logic_error{"socket does not have an addr object"};
        }
        virtual void close() noexcept = 0;

    	[[nodiscard]] virtual fd_t native_handle() const {
    		throw std::logic_error{"socket does not have a native handle"};
    	}
    };
}