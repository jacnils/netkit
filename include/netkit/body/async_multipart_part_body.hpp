#pragma once

#include <netkit/body/basic_async_body.hpp>
#include <netkit/http/async_multipart_reader.hpp>

namespace netkit::body {
	class async_multipart_part_body : public basic_async_body {
	public:
		async_multipart_part_body(http::utility::async_multipart_reader& reader) : reader_(reader) {}
		[[nodiscard]] io::task<read_result> read(char* buffer, size_t size) noexcept override;
	private:
		http::utility::async_multipart_reader& reader_;
	};
}