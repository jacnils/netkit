#include <netkit/body/async_multipart_part_body.hpp>

netkit::io::task<netkit::body::read_result>
netkit::body::async_multipart_part_body::read(char* buffer, std::size_t max_bytes) noexcept {
	co_return co_await reader_.read_part(buffer, max_bytes);
}