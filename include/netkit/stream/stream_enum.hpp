#pragma once

namespace netkit::stream {
	enum class stream_status {
		success,
		closed,
		error
	};

	struct stream_result {
		std::size_t bytes{};
		stream_status status{stream_status::closed};
	};
}