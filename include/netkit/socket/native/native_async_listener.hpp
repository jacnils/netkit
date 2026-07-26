#pragma once

#include <sys/socket.h>
#include <memory>

#include <netkit/io/io_context.hpp>
#include <netkit/io/task.hpp>
#include <netkit/socket/native/basic_native_async_listener.hpp>
#include <netkit/socket/native/basic_native_async_sock.hpp>

namespace netkit::sock::native {
#ifndef NETKIT_WINDOWS
constexpr fd_t INVALID_SOCKET = -1;
#endif

class NETKIT_API native_async_listener  : public basic_native_async_listener {
public:
	native_async_listener(io::io_context& ctx, const addr& address, type t = type::tcp, opt opts = opt::no_reuse_addr | opt::no_blocking);
	~native_async_listener() override;

	void bind() override;
	void unbind() override;
	void listen(int backlog) override;
	void listen() override;

	[[nodiscard]] io::task<std::unique_ptr<basic_native_async_sock>> accept() override;

	void close() noexcept override;

	[[nodiscard]] const addr& get_local_endpoint() const override;

	[[nodiscard]] fd_t native_handle() const override;
	void set_sock_opts(opt opts) const;

private:
	io::io_context& context_;

	addr addr_;
	type type_;

	fd_t sockfd_{INVALID_SOCKET};

	opt opts_;

	bool bound_{false};
	bool listening_{false};

	sockaddr_storage sa_storage_{};

	void prep_sa();

	[[nodiscard]]
	const sockaddr* get_sa() const;

	[[nodiscard]]
	socklen_t get_sa_len() const;
};

}