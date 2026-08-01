#pragma once

#include <netkit/socket/native/basic_native_sync_listener.hpp>

#include <memory>

#ifndef NETKIT_DKP

namespace netkit::uds {

class uds_stream;

class uds_server {
public:
	uds_server(sock::addr addr);

	~uds_server();

	void bind();
	void listen();
	void listen(int backlog);

	std::unique_ptr<uds_stream> accept();

	void close() noexcept;

	const sock::addr& get_local_endpoint() const noexcept;
private:
	sock::addr addr_;
	std::unique_ptr<sock::native::basic_native_sync_listener> listener_;
};

}

#endif