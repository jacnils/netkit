#pragma once

#include <netkit/definitions.hpp>

#ifdef NETKIT_UNIX
#include <sys/socket.h>
#elifdef NETKIT_WINDOWS
#include <ws2tcpip.h>
#endif

#include <netkit/socket/native/basic_native_sync_listener.hpp>

namespace netkit::socket::native {
class NETKIT_API native_sync_listener : public basic_native_sync_listener {
public:
	void set_sock_opts(opt opts) const;
	native_sync_listener(const addr& address, type t = type::tcp, opt opts = opt::reuse_addr|opt::blocking);
	~native_sync_listener() override = default;

	void bind() override;
	void bind(const addr& addr) override;
	void unbind() override;

	void listen(int backlog) override;
	void listen() override;

	std::unique_ptr<basic_native_sync_socket> accept() override;

	void close() noexcept override;

	[[nodiscard]] const addr& get_local_endpoint() const override;

	[[nodiscard]] fd_t native_handle() const override;
private:
	addr addr_;
	type type_;

	fd_t sockfd_{};

	bool bound_{false};
	bool listening_{false};

	opt opts_;
};

}