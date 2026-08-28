#pragma once

#include <netkit/io/task.hpp>
#include <netkit/stream/async_socket_stream.hpp>
#include <netkit/socket/native/native_async_socket.hpp>

#include <memory>
#include <vector>
#include <string_view>

namespace netkit::tcp {

class NETKIT_API async_tcp_stream : public stream::basic_async_stream {
public:
	using basic_async_stream::write;
	using basic_async_stream::read;
	async_tcp_stream(io::io_context& ctx, const socket::addr& addr) : stream_(std::make_unique<socket::native::native_async_socket>(ctx, addr, socket::type::tcp)) {}
	async_tcp_stream(std::unique_ptr<socket::native::basic_native_async_socket> socket) : stream_(std::move(socket)) {}
	~async_tcp_stream() override;

	[[nodiscard]] netkit::io::task<> connect() const;
	netkit::io::task<stream::stream_result> read(std::span<std::byte> buffer) override;
	netkit::io::task<stream::stream_result> write(std::span<const std::byte> buffer) override;

	void close() noexcept override;
	[[nodiscard]] socket::addr peer() const;

	stream::async_socket_stream& stream();
private:
	stream::async_socket_stream stream_;
};

}