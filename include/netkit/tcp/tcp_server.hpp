#pragma once

#include <netkit/socket/native/basic_native_sync_listener.hpp>

#include <memory>

namespace netkit::tcp {

class tcp_stream;

class tcp_server {
public:
	tcp_server(sock::addr addr);

	~tcp_server();

	void bind();
	void listen();
	void listen(int backlog);

	std::unique_ptr<tcp_stream> accept();

	void close() noexcept;

	const sock::addr& get_local_endpoint() const noexcept;
private:
	sock::addr addr_;
	std::unique_ptr<sock::native::basic_native_sync_listener> listener_;
};

}