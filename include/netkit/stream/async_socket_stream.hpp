#pragma once

#include <netkit/socket/native/native_async_sock.hpp>
#include <netkit/stream/basic_async_stream.hpp>

namespace netkit::stream {

class NETKIT_API async_socket_stream : public basic_async_stream {
public:
	explicit async_socket_stream(std::unique_ptr<sock::native::basic_native_async_sock> socket) : socket_(std::move(socket)) {}

	[[nodiscard]] io::task<> connect() const;
	[[nodiscard]] io::task<stream_result> read(std::span<std::byte> buffer) override;
	[[nodiscard]] io::task<stream_result> write(std::span<const std::byte> buffer) override;

	void close() noexcept override;

	[[nodiscard]] sock::addr peer() const;
private:
	std::unique_ptr<sock::native::basic_native_async_sock> socket_;
};

}