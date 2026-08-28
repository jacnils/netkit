#pragma once

#include <netkit/socket/native/basic_native_async_listener.hpp>
#include <netkit/io/task.hpp>

#include <memory>

#ifndef NETKIT_DKP

namespace netkit::uds {

class async_uds_stream;

class async_uds_server {
public:
	async_uds_server(
		io::io_context& ctx,
		socket::addr addr
	);

	~async_uds_server();

	void bind();

	void listen();
	void listen(int backlog);

	io::task<std::unique_ptr<async_uds_stream>> accept();

	void close() noexcept;

	[[nodiscard]] const socket::addr& get_local_endpoint() const noexcept;
private:
	socket::addr addr_;
	std::unique_ptr<socket::native::basic_native_async_listener> listener_;
};

}

#endif