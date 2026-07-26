#pragma once

#include <netkit/io/task.hpp>
#include <netkit/stream/async_socket_stream.hpp>

#include <memory>
#include <vector>
#include <string_view>

namespace netkit::tcp {

class async_tcp_connection {
public:
	async_tcp_connection(io::io_context& ctx, const sock::addr& addr) : stream_(std::make_unique<sock::native::native_async_sock>(ctx, addr, sock::type::tcp)) {}
	async_tcp_connection(std::unique_ptr<sock::native::basic_native_async_sock> socket) : stream_(std::move(socket)) {}
	~async_tcp_connection();

	netkit::io::task<> connect() const;
	netkit::io::task<stream::stream_result> read(std::span<std::byte> buffer);
	netkit::io::task<stream::stream_result> write(std::span<const std::byte> buffer);
	netkit::io::task<stream::stream_result> write_all(std::span<const std::byte> buffer);
	netkit::io::task<stream::stream_result> write_all(std::string_view data);
	netkit::io::task<std::vector<std::byte>> read_all(std::size_t max_bytes = 16 * 1024 * 1024);
	netkit::io::task<std::string> read_all_string();

	void close() noexcept;
	[[nodiscard]] sock::addr peer() const;

	stream::async_socket_stream& stream();

private:
	stream::async_socket_stream stream_;
};

}