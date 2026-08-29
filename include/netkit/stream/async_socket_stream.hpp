#pragma once

#include <netkit/socket/native/basic_native_async_socket.hpp>
#include <netkit/stream/basic_async_stream.hpp>
#include <netkit/socket/addr.hpp>
#include <memory>

namespace netkit::stream {

class NETKIT_API async_socket_stream : public basic_async_stream {
public:
	using basic_async_stream::write;
	using basic_async_stream::read;

	explicit async_socket_stream(std::unique_ptr<socket::native::basic_native_async_socket> socket) : socket_(std::move(socket)) {}

	[[nodiscard]] io::task<> connect() const;
	[[nodiscard]] io::task<stream_result> read(std::span<std::byte> buffer) override;
	[[nodiscard]] io::task<stream_result> write(std::span<const std::byte> buffer) override;

	void close() noexcept override;

	[[nodiscard]] bool is_open() const noexcept override;

	[[nodiscard]] socket::addr peer() const;

	std::optional<socket::addr> get_addr() override {
		return socket_->get_addr();
	}
private:
	std::unique_ptr<socket::native::basic_native_async_socket> socket_;
};

}