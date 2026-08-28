#pragma once

#include <netkit/socket/native/native_sync_listener.hpp>
#include <netkit/socket/addr.hpp>
#include <netkit/stream/basic_stream.hpp>
#include <netkit/stream/socket_stream.hpp>

#include <memory>
#include <vector>

namespace netkit::tcp {

class tcp_stream : public stream::basic_stream {
public:
	using basic_stream::write;
	using basic_stream::read;

	tcp_stream(const socket::addr& addr) : stream_(std::make_unique<socket::native::native_sync_socket>(addr, socket::type::tcp)) {}
	tcp_stream(std::unique_ptr<socket::native::basic_native_sync_socket> socket) : stream_(std::move(socket)) {}

	~tcp_stream() override;

	void connect();

	stream::stream_result read(std::span<std::byte> buffer) override;
	stream::stream_result write(std::span<const std::byte> buffer) override;

	void close() noexcept override;

	[[nodiscard]] socket::addr peer() const;
	std::optional<socket::addr> get_addr() override {
		return stream_.get_addr();
	}

	stream::socket_stream& stream();

private:
	stream::socket_stream stream_;
};

}