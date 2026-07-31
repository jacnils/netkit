#pragma once

#include <netkit/stream/basic_stream.hpp>
#include <netkit/socket/native/native_sync_sock.hpp>

namespace netkit::stream {

class socket_stream : public basic_stream {
public:
	using basic_stream::write;
	using basic_stream::read;

	explicit socket_stream(std::unique_ptr<sock::native::basic_native_sync_sock> socket)
	: socket_(std::move(socket)) {}

	void connect();

	[[nodiscard]] stream_result read(std::span<std::byte> buffer) override;
	[[nodiscard]] stream_result write(std::span<const std::byte> buffer) override;

	void close() noexcept override;

	[[nodiscard]] sock::addr peer() const;

	std::optional<sock::addr> get_addr() override {
		return socket_->get_addr();
	}
private:
	std::unique_ptr<sock::native::basic_native_sync_sock> socket_;
};

}