#pragma once

#include <netkit/socket/native/basic_native_async_listener.hpp>
#include <netkit/io/task.hpp>

#include <memory>

namespace netkit::tcp {

class async_tcp_stream;

class async_tcp_server {
public:
	async_tcp_server(
		io::io_context& ctx,
		socket::addr addr
	);

	~async_tcp_server();

	void bind();

	void listen();
	void listen(int backlog);

	io::task<std::unique_ptr<async_tcp_stream>> accept();

	void close() noexcept;

	[[nodiscard]] const socket::addr& get_local_endpoint() const noexcept;
private:
	socket::addr addr_;
	std::unique_ptr<socket::native::basic_native_async_listener> listener_;
};

}