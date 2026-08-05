#include <algorithm>
#include <cstring>
#ifndef NOMINMAX
#define NOMINMAX // some windows shit
#endif
#include <netkit/body/chunked_body.hpp>

bool netkit::body::chunked_body::fill_buffer(std::size_t min_bytes) noexcept {
    while (buffer_.size() < min_bytes) {
        std::array<std::byte, 8192> temp{};

        auto result = stream_.read(
            std::span<std::byte>(temp)
        );

        if (result.status != stream::stream_status::success)
            return false;

        if (result.bytes == 0)
            return false;

        buffer_.append(
            reinterpret_cast<const char*>(temp.data()),
            result.bytes
        );
    }

    return true;
}

std::size_t netkit::body::chunked_body::read_raw(char* out, const std::size_t bytes) noexcept {
    if (buffer_.empty()) {
        if (!fill_buffer(1))
            return 0;
    }

    auto n = std::min(bytes, buffer_.size());

    std::memcpy(out, buffer_.data(), n);
    buffer_.erase(0, n);

    return n;
}

std::string netkit::body::chunked_body::read_line() noexcept {
    while (true) {
        auto pos = buffer_.find("\r\n");

        if (pos != std::string::npos) {
            std::string line = buffer_.substr(0, pos);
            buffer_.erase(0, pos + 2);
            return line;
        }

        if (!fill_buffer(buffer_.size() + 1))
            return std::string{};
    }
}

netkit::body::read_result netkit::body::chunked_body::read(char* out, std::size_t max_bytes) noexcept {
    if (max_bytes == 0)
        return read_result{read_status::ok, 0};

    while (true) {
        if (state_ == state::done)
            return read_result{read_status::eof, 0};

        if (state_ == state::read_size) {
            auto line = read_line();

            if (line.empty())
                return read_result{read_status::error, 0};

            if (auto pos = line.find(';'); pos != std::string::npos)
                line.resize(pos);

            try {
                chunk_remaining_ = std::stoull(line, nullptr, 16);
            } catch (...) {
                return read_result{read_status::error, 0};
            }

            if (chunk_remaining_ == 0) {
                state_ = state::done;
                return read_result{read_status::eof, 0};
            }

            state_ = state::read_data;
        }

        if (state_ == state::read_data) {
            auto want = std::min(max_bytes, chunk_remaining_);

            auto n = read_raw(out, want);

            if (n == 0)
                return read_result{read_status::eof, 0};

            chunk_remaining_ -= n;

            if (chunk_remaining_ == 0)
                state_ = state::consume_crlf;

            return read_result{
                read_status::ok,
                n
            };
        }

        if (state_ == state::consume_crlf) {
            char crlf[2];

            auto n = read_raw(crlf, 2);

            if (n != 2 || crlf[0] != '\r' || crlf[1] != '\n') {
                return read_result{
                    read_status::error,
                    0
                };
            }

            state_ = state::read_size;
        }
    }
}