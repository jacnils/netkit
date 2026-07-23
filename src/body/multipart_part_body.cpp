#include <netkit/body/multipart_part_body.hpp>

netkit::body::read_result netkit::body::multipart_part_body::read(char* buffer, std::size_t max_bytes) noexcept {
	return reader_.read_part(buffer, max_bytes);
}