#pragma once

#include <netkit/stream/basic_stream.hpp>
#include <netkit/socket/native/native_sync_sock.hpp>

namespace netkit::stream {

class socket_stream : public basic_stream {
public:
	explicit socket_stream(std::unique_ptr<sock::native::basic_native_sync_sock> socket)
	: socket_(std::move(socket)) {}

	void connect() {
		if (!socket_)
			throw std::logic_error("socket closed");

		socket_->connect();
	}

	[[nodiscard]] stream_result read(std::span<std::byte> buffer) override {
		if (!socket_)
			throw std::logic_error("socket closed");

		auto result = socket_->recv(
			buffer.data(),
			buffer.size()
		);

		if (result == 0) {
			return {
				0,
				stream_status::eof
			};
		}

		if (result < 0) {
			return {
				0,
				stream_status::error
			};
		}

		return {
			static_cast<std::size_t>(result),
			stream_status::success
		};
	}

	[[nodiscard]] stream_result write(std::span<const std::byte> buffer) override {
		if (!socket_)
			throw std::logic_error("socket closed");

		auto result = socket_->send(
			buffer.data(),
			buffer.size()
		);

		if (result < 0) {
			return {
				0,
				stream_status::error
			};
		}

		return {
			static_cast<std::size_t>(result),
			stream_status::success
		};
	}

	void close() noexcept override {
		if (socket_) {
			socket_->close();
			socket_.reset();
		}
	}

	[[nodiscard]] sock::addr peer() const {
		if (!socket_)
			throw std::logic_error("socket closed");

		return socket_->get_peer();
	}
private:
	std::unique_ptr<sock::native::basic_native_sync_sock> socket_;
};

}