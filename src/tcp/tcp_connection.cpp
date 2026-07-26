#include <netkit/tcp/tcp_connection.hpp>

netkit::tcp::tcp_connection::~tcp_connection() {
	this->close();
}

void netkit::tcp::tcp_connection::connect() {
	stream_.connect();
}

netkit::stream::stream_result netkit::tcp::tcp_connection::read(std::span<std::byte> buffer) {
	return stream_.read(buffer);
}

netkit::stream::stream_result netkit::tcp::tcp_connection::write(std::span<const std::byte> buffer) {
	return stream_.write(buffer);
}

netkit::stream::stream_result netkit::tcp::tcp_connection::write_all(std::span<const std::byte> buffer) {
	std::size_t total = 0;

	while (total < buffer.size()) {
		auto result = this->write(buffer.subspan(total));

		if (result.status != stream::stream_status::success)
			return {
				result.bytes + total,
				result.status
			};

		total += result.bytes;
	}

	return {
		total, netkit::stream::stream_status::success
	};
}

netkit::stream::stream_result netkit::tcp::tcp_connection::write_all(std::string_view data) {
	return write_all(
		std::as_bytes(
			std::span(data.data(), data.size())
		)
	);
}

std::vector<std::byte> netkit::tcp::tcp_connection::read_all(std::size_t max_bytes) {
	std::vector<std::byte> result;

	std::size_t total = 0;

	std::array<std::byte, 8192> buffer{};

	while (total < max_bytes) {
		auto res = read(buffer);

		if (res.status == netkit::stream::stream_status::eof)
			break;

		if (res.status != netkit::stream::stream_status::success)
			throw std::runtime_error("read failed");

		auto amount = std::min(res.bytes, max_bytes - total);

		result.insert(
			result.end(),
			buffer.begin(),
			buffer.begin() + amount
		);

		total += amount;

		if (amount != res.bytes)
			throw std::length_error("read_all exceeded maximum size");
	}

	return result;
}

std::string netkit::tcp::tcp_connection::read_all_string() {
	auto data = this->read_all();

	return {
		reinterpret_cast<const char*>(data.data()),
		data.size()
	};
}

void netkit::tcp::tcp_connection::close() noexcept {
	stream_.close();
}

netkit::sock::addr netkit::tcp::tcp_connection::peer() const {
	return stream_.peer();
}

netkit::stream::socket_stream& netkit::tcp::tcp_connection::stream() {
	return stream_;
}