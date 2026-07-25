#pragma once

#include <netkit/socket/native/basic_native_sync_sock.hpp>
#include <algorithm>
#include <cstring>
#include <netkit/body/basic_body.hpp>
#include <optional>
#include <string>

namespace netkit::body {

class NETKIT_API stream_body : public basic_body {
public:
	stream_body(sock::native::basic_native_sync_sock& socket,
				std::optional<std::size_t> length,
				std::string initial = {})
		: socket_(socket),
		  remaining_(length),
		  buffer_(std::move(initial))
	{}

	read_result read(char* buffer, std::size_t max_bytes) noexcept override;

	[[nodiscard]] std::optional<std::size_t> size() const override {
		return remaining_;
	}

private:
	sock::native::basic_native_sync_sock& socket_;
	std::optional<std::size_t> remaining_;
	std::string buffer_;
	std::string overflow_;
};

}