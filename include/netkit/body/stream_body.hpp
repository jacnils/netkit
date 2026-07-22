#pragma once

#include <algorithm>
#include <cstring>
#include <optional>
#include <string>

#include <netkit/body/basic_body.hpp>
#include <netkit/sock/basic_sync_sock.hpp>

namespace netkit::body {

class NETKIT_API stream_body : public basic_body {
public:
	stream_body(sock::basic_sync_sock& socket,
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
	sock::basic_sync_sock& socket_;
	std::optional<std::size_t> remaining_;
	std::string buffer_;
	std::string overflow_;
};

}