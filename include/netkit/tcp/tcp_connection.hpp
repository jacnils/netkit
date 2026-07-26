#pragma once

#include <netkit/socket/native/native_sync_listener.hpp>
#include <netkit/socket/addr.hpp>
#include <netkit/stream/basic_stream.hpp>
#include <netkit/stream/socket_stream.hpp>

#include <memory>
#include <vector>

namespace netkit::tcp {

class tcp_connection {
public:
	tcp_connection(const sock::addr& addr) : stream_(std::make_unique<sock::native::native_sync_sock>(addr, sock::type::tcp)) {}
	tcp_connection(std::unique_ptr<sock::native::basic_native_sync_sock> socket) : stream_(std::move(socket)) {}

	~tcp_connection();

	void connect();

	stream::stream_result read(std::span<std::byte> buffer);
	stream::stream_result write(std::span<const std::byte> buffer);
	stream::stream_result write_all(std::span<const std::byte> buffer);
	stream::stream_result write_all(std::string_view data);
	std::vector<std::byte> read_all(std::size_t max_bytes = 16 * 1024 * 1024);
	std::string read_all_string();

	void close() noexcept;

	[[nodiscard]] sock::addr peer() const;

	stream::socket_stream& stream();

private:
	stream::socket_stream stream_;
};

}