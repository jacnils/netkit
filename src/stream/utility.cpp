#include <netkit/stream/utility.hpp>
#include <netkit/except.hpp>

netkit::stream::ReadUntilData netkit::stream::read_until(stream::basic_stream& client_sock, const std::string& delimiter) {
    ReadUntilData ret;
    char buffer[4096];

    while (true) {
        const auto [bytes, status] =
            client_sock.read(buffer, sizeof(buffer));

        if (status == stream::stream_status::error) {
            throw socket_error{"error occurred"};
        }

        ret.second.append(buffer, bytes);

        const auto pos = ret.second.find(delimiter);

        if (pos != std::string::npos) {
            ret.first = ret.second.substr(0, pos);
            ret.second = ret.second.substr(pos + delimiter.size());
            break;
        }

        if (status == stream::stream_status::eof) {
            break;
        }
    }

    return ret;
}

netkit::io::task<netkit::stream::ReadUntilData>
netkit::stream::read_until(stream::basic_async_stream& client_sock, const std::string& delimiter) {
    ReadUntilData ret;
    char buffer[4096];

    while (true) {
        const auto [bytes, status] = co_await client_sock.read(buffer, sizeof(buffer));

        if (status == stream::stream_status::error) {
            throw socket_error{"error occurred"};
        }

        ret.second.append(buffer, bytes);

        const auto pos = ret.second.find(delimiter);

        if (pos != std::string::npos) {
            ret.first = ret.second.substr(0, pos);
            ret.second = ret.second.substr(pos + delimiter.size());
            break;
        }

        if (status == stream::stream_status::eof) {
            break;
        }
    }

    co_return ret;
}
