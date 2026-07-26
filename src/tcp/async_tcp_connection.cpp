#include <netkit/tcp/async_tcp_connection.hpp>

netkit::tcp::async_tcp_connection::~async_tcp_connection() {
	this->close();
}

netkit::io::task<void> netkit::tcp::async_tcp_connection::connect() const {
	co_await stream_.connect();
}

netkit::io::task<netkit::stream::stream_result>
netkit::tcp::async_tcp_connection::read(std::span<std::byte> buffer) {
	co_return co_await stream_.read(buffer);
}

netkit::io::task<netkit::stream::stream_result>
netkit::tcp::async_tcp_connection::write(std::span<const std::byte> buffer) {
	co_return co_await stream_.write(buffer);
}

netkit::io::task<netkit::stream::stream_result>
netkit::tcp::async_tcp_connection::write_all(std::span<const std::byte> buffer) {
	std::size_t total = 0;

	while (total < buffer.size()) {
		auto result = co_await write(
			buffer.subspan(total)
		);

		if (result.status != stream::stream_status::success)
			co_return result;

		total += result.bytes;
	}

	co_return stream::stream_result{
		total,
		stream::stream_status::success
	};
}

netkit::io::task<netkit::stream::stream_result>
netkit::tcp::async_tcp_connection::write_all(std::string_view data) {
	co_return co_await this->write_all(std::as_bytes(std::span(data.data(), data.size())));
}

netkit::io::task<std::vector<std::byte>>
netkit::tcp::async_tcp_connection::read_all(std::size_t max_bytes) {
	std::vector<std::byte> result;

	std::array<std::byte, 8192> buffer{};

	std::size_t total = 0;

	while (total < max_bytes) {
		auto res = co_await read(buffer);

		if (res.status == stream::stream_status::eof)
			break;

		if (res.status != stream::stream_status::success)
			throw std::runtime_error("read failed");

		result.insert(
			result.end(),
			buffer.begin(),
			buffer.begin() + res.bytes
		);

		total += res.bytes;
	}

	co_return result;
}

netkit::io::task<std::string>
netkit::tcp::async_tcp_connection::read_all_string() {
	auto data = co_await this->read_all();

	co_return std::string{
		reinterpret_cast<const char*>(data.data()),
		data.size()
	};
}

void netkit::tcp::async_tcp_connection::close() noexcept {
	stream_.close();
}

netkit::sock::addr netkit::tcp::async_tcp_connection::peer() const {
	return stream_.peer();
}

netkit::stream::async_socket_stream& netkit::tcp::async_tcp_connection::stream() {
	return stream_;
}