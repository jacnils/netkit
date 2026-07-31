#pragma once

#include <netkit/body/basic_body.hpp>
#include <netkit/http/multipart_reader.hpp>

namespace netkit::body {
	class multipart_part_body : public basic_body {
	public:
		multipart_part_body(http::utility::multipart_reader& reader) : reader_(reader) {}
		[[nodiscard]] read_result read(char* buffer, size_t size) noexcept override;
	private:
		http::utility::multipart_reader& reader_;
	};
}