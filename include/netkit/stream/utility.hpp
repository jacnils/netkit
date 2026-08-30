#pragma once

#include <utility>
#include <string>
#include <memory>

#include <netkit/stream/basic_stream.hpp>
#include <netkit/stream/basic_async_stream.hpp>

namespace netkit::stream {
    typedef std::string Overflow;
    typedef std::string Data;
    typedef std::pair<Overflow, Data> ReadUntilData;

    ReadUntilData read_until(stream::basic_stream& client_sock, const std::string& delimiter);
    netkit::io::task<ReadUntilData> read_until(stream::basic_async_stream& client_sock, const std::string& delimiter);
}
