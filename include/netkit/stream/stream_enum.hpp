#pragma once

#include <netkit/export.hpp>
#include <cstdint>

namespace netkit::stream {
	enum class stream_status {
		success,
		closed,
		error,
		eof
	};

	struct NETKIT_API stream_result {
		std::size_t bytes{};
		stream_status status{stream_status::closed};
	};
}