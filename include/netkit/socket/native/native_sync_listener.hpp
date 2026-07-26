#pragma once

#include <sys/socket.h>
#include <netkit/socket/native/basic_native_sync_listener.hpp>

namespace netkit::sock::native {
#ifndef NETKIT_WINDOWS
constexpr fd_t INVALID_SOCKET = -1;
#endif

class NETKIT_API native_sync_listener
	: public basic_native_sync_listener
{
public:
	void set_sock_opts(opt opts) const;
	native_sync_listener(const addr& address, type t = type::tcp, opt opts = opt::no_reuse_addr);
	~native_sync_listener() override = default;

	void bind() override;
	void unbind() override;

	void listen(int backlog) override;
	void listen() override;

	std::unique_ptr<basic_native_sync_sock> accept() override;

	void close() noexcept override;

	[[nodiscard]] const addr& get_local_endpoint() const override;

	[[nodiscard]] fd_t native_handle() const override;
private:
	addr addr_;
	type type_;

	fd_t sockfd_{INVALID_SOCKET};

	bool bound_{false};
	bool listening_{false};

	opt opts_;

	sockaddr_storage sa_storage_{};

	void prep_sa();

	[[nodiscard]]
	const sockaddr* get_sa() const;

	[[nodiscard]]
	socklen_t get_sa_len() const;
};

}