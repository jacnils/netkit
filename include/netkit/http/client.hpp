#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include <netkit/http/predefined.hpp>
#include <netkit/body/basic_body.hpp>
#include <netkit/socket/addr.hpp>
#include <netkit/stream/basic_stream.hpp>

#include <netkit/stream/wolfssl/tls_stream.hpp>
#include <netkit/body/stream_body.hpp>
#include <netkit/body/chunked_body.hpp>
#include <netkit/except.hpp>

#ifdef NETKIT_SSL
#include <netkit/tcp/tcp_stream.hpp>
#endif

namespace netkit::http {
    class header_name {
    public:
        header_name() = default;

        explicit header_name(std::string name)
            : value_(std::move(name))
        {
            normalize(value_);
        }

        bool operator==(const header_name& rhs) const {
            return value_ == rhs.value_;
        }

        bool operator==(std::string_view rhs) const {
            return equals_ignore_case(value_, rhs);
        }

        [[nodiscard]]
        std::size_t size() const {
            return value_.size();
        }

        [[nodiscard]]
        bool empty() const {
            return value_.empty();
        }

        [[nodiscard]]
        const std::string& value() const {
            return value_;
        }

        header_name& operator=(const std::string& first) {
            value_ = first;
            normalize(value_);
            return *this;
        }

    private:
        static constexpr void normalize(std::string& str) {
            for (char& c : str) {
                if (c >= 'A' && c <= 'Z')
                    c += 'a' - 'A';
            }
        }

        static constexpr bool equals_ignore_case(
            std::string_view lhs,
            std::string_view rhs)
        {
            if (lhs.size() != rhs.size())
                return false;

            for (std::size_t i = 0; i < lhs.size(); ++i) {
                if (normalize_char(lhs[i]) != normalize_char(rhs[i]))
                    return false;
            }

            return true;
        }

        static constexpr char normalize_char(char c) {
            return (c >= 'A' && c <= 'Z')
                ? c + ('a' - 'A')
                : c;
        }

        std::string value_;
    };

    struct header {
        header_name name;
        std::string value;

        header() = default;
        header(std::string first, std::string second)
            : name(std::move(first)),
              value(std::move(second))
        {}
    };

    class headers {
    public:
        using value_type = header;
        using container_type = std::vector<header>;
        using iterator = container_type::iterator;
        using const_iterator = container_type::const_iterator;

        headers() = default;

        void add(std::string name, std::string value) {
            headers_.emplace_back(
                std::move(name),
                std::move(value)
            );
        }

        void add(header h) {
            add(h.name.value(), std::move(h.value));
        }

        headers(const headers& headers) {
            for (const auto& it : headers) {
                this->add(it);
            }
        }

        [[nodiscard]] bool contains(const std::string& name) const {
            return find(name) != nullptr;
        }

        [[nodiscard]] const header* find(const std::string& name) const {
            for (const auto& h : headers_) {
                if (h.name == name)
                    return &h;
            }

            return nullptr;
        }

        header* find(const std::string& name) {
            for (auto& h : headers_) {
                if (h.name == name)
                    return &h;
            }

            return nullptr;
        }

        [[nodiscard]] std::string value(const std::string& name) const {
            if (const auto* h = find(name))
                return h->value;

            return {};
        }

        iterator begin() noexcept { return headers_.begin(); }
        [[nodiscard]] const_iterator begin() const noexcept { return headers_.begin(); }

        iterator end() noexcept { return headers_.end(); }
        [[nodiscard]] const_iterator end() const noexcept { return headers_.end(); }

        [[nodiscard]] size_t size() const noexcept {
            return headers_.size();
        }

        [[nodiscard]] bool empty() const noexcept {
            return headers_.empty();
        }

        header& operator[](size_t index) {
            return headers_[index];
        }

        const header& operator[](size_t index) const {
            return headers_[index];
        }

    private:
        container_type headers_;
    };

    struct response {
        int status_code{};
        netkit::http::headers headers;
        std::unique_ptr<body::basic_body> body;
    };

    enum class scheme {
#ifdef NETKIT_SSL
        https,
#endif
        http
    };

    class client {
        socket::addr addr;
        scheme scheme_;
        std::unique_ptr<stream::basic_stream> stream;

        struct settings {
            std::string user_agent = "netkit/0.1";
            std::string accept = "*/*";
            std::string content_type = "application/octet-stream";
            bool close = false;
        } settings;

        static std::pair<std::string, std::string> read_until(
            const std::unique_ptr<stream::basic_stream>& client_sock,
            const std::string& delimiter)
        {
            std::pair<std::string, std::string> ret;
            char buffer[4096];

            while (true) {
                const auto [bytes, status] =
                    client_sock->read(buffer, sizeof(buffer));

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

        [[nodiscard]] static headers
        get_headers(const std::string& header_part) {
            headers headers_map;

            auto trim = [](std::string& s) {
                auto start = s.find_first_not_of(" \t");

                if (start == std::string::npos) {
                    s.clear();
                    return;
                }

                auto end = s.find_last_not_of(" \t");

                s = s.substr(start, end - start + 1);
            };

            auto lowercase = [](std::string& s) {
                std::ranges::transform(
                    s,
                    s.begin(),
                    [](unsigned char c) {
                        return static_cast<char>(std::tolower(c));
                    }
                );
            };

            std::istringstream hs(header_part);
            std::string line;

            while (std::getline(hs, line)) {
                if (!line.empty() && line.back() == '\r')
                    line.pop_back();

                if (line.empty())
                    break;

                auto colon = line.find(':');

                if (colon == std::string::npos)
                    continue;

                auto key = line.substr(0, colon);
                auto value = line.substr(colon + 1);

                trim(key);
                trim(value);

                lowercase(key);

                headers_map.add(key, value);
            }

            return headers_map;
        }

        void connect() {
            if (!stream || !stream->is_open()) {
#ifdef NETKIT_SSL
                if (scheme_ == scheme::https) {
                    auto sockstream = std::make_unique<tcp::tcp_stream>(addr);
                    sockstream->connect();

                    auto tls_sockstream = std::make_unique<stream::tls_stream>(std::move(sockstream));
                    tls_sockstream->perform_handshake();

                    stream = std::move(tls_sockstream);

                    return;
                }
#endif
                auto sockstream = std::make_unique<tcp::tcp_stream>(addr);
                sockstream->connect();
                stream = std::move(sockstream);
            }
        }

        static std::string get_method_string(method method) {
            std::stringstream ss;
            switch (method) {
            case method::GET:
                ss << "GET"; break;
            case method::HEAD:
                ss << "HEAD"; break;
            case method::POST:
                ss << "POST"; break;
            case method::PUT:
                ss << "PUT"; break;
            case method::DELETE:
                ss << "DELETE"; break;
            case method::CONNECT:
                ss << "CONNECT"; break;
            case method::OPTIONS:
                ss << "OPTIONS"; break;
            case method::TRACE:
                ss << "TRACE"; break;
            case method::PATCH:
                ss << "PATCH"; break;
            case method::undefined:
                throw std::logic_error("undefined method");
            }
            return ss.str();
        }

        void set_headers(netkit::http::headers& h, const std::unique_ptr<body::basic_body>& body) const {
            try {
                if (addr.get_port() != 80 && addr.get_port() != 443)
                    h.add("Host", addr.get_hostname() + ":" + std::to_string(addr.get_port()));
                else
                    h.add("Host", addr.get_hostname());
            } catch (...) {
                if (addr.get_port() != 80 && addr.get_port() != 443)
                    h.add("Host", addr.get_ip() + ":" + std::to_string(addr.get_port()));
                else
                    h.add("Host", addr.get_ip());
            }

            if (!settings.user_agent.empty()) {
                h.add("User-Agent", settings.user_agent);
            }

            if (!settings.accept.empty()) {
                h.add("Accept", settings.accept);
            }

            if (settings.close) {
                h.add("Connection", "close");
            }

            if (body && body->size().has_value()) {
                h.add("Content-Length", std::to_string(body->size().value()));
            } else if (body) {
                throw std::logic_error("cannot use this body type as of now");
            }

            if (body && !body->empty()) {
                h.add("Content-Type", settings.content_type);
            }
        }

        static int parse_status_code(std::string_view status_line) {
            const auto first_space = status_line.find(' ');

            if (first_space == std::string_view::npos)
                throw std::logic_error{"invalid HTTP status line"};

            const auto code_start = first_space + 1;
            const auto second_space = status_line.find(' ', code_start);

            if (second_space == std::string_view::npos)
                throw std::logic_error{"invalid HTTP status line"};

            const auto code_str = status_line.substr(
                code_start,
                second_space - code_start
            );

            int code;

            const auto [ptr, ec] = std::from_chars(
                code_str.data(),
                code_str.data() + code_str.size(),
                code
            );

            if (ec != std::errc{} || ptr != code_str.data() + code_str.size())
                throw std::logic_error{"invalid HTTP status code"};

            return code;
        }
    public:
        explicit client(const socket::addr& addr, const scheme scheme) : addr(addr), scheme_(scheme) {}

        response request(const std::string& method, const std::string& path, const std::unique_ptr<body::basic_body>& body, const headers& headers = {}) {
            std::stringstream ss;

            ss << method;

            if (path.empty()) {
                throw std::logic_error("path is empty");
            }

            if (path.at(0) != '/') {
                throw std::logic_error("path must start with /");
            }

            ss << " " << path << " HTTP/1.1" << "\r\n";

            netkit::http::headers h = headers;

            set_headers(h, body);

            for (auto& it : h) {
                ss << it.name.value() << ": " << it.value << "\r\n";
            }

            ss << "\r\n";

            this->connect();

            stream->write_all(ss.str());

            if (body && !body->empty()) {
                char buf[4096];
                size_t bytes_read = 0;

                while (bytes_read < body->size()) {
                    const size_t remaining = *body->size() - bytes_read;
                    const size_t to_read = std::min(sizeof(buf), remaining);

                    auto res = stream->read(buf, to_read);

                    if (res.status != stream::stream_status::success &&
                        res.status != stream::stream_status::eof) {
                        throw std::logic_error("cannot read from stream");
                        }

                    if (res.bytes == 0)
                        break;

                    stream->write_all(buf, res.bytes);
                    bytes_read += res.bytes;
                }

                if (bytes_read != body->size()) {
                    throw std::logic_error("unexpected end of stream");
                }
            }

            auto [header_data, overflow] = read_until(stream, "\r\n\r\n");

            const auto line_end = header_data.find_first_of("\r\n");

            if (line_end == std::string::npos)
                throw std::logic_error{"invalid HTTP response"};

            const auto status_line = std::string_view{
                header_data.data(),
                line_end
            };

            const auto headers_data = std::string_view{
                header_data.data() + line_end + 2,
                header_data.size() - line_end - 2
            };

            response resp;
            resp.headers = get_headers(headers_data.data());
            resp.status_code = parse_status_code(status_line);

            if (resp.headers.contains("transfer-encoding") && resp.headers.value("transfer-encoding") == "chunked") {
                resp.body = std::make_unique<netkit::body::chunked_body>(*stream, std::move(overflow));
            } else if (resp.headers.contains("content-length")) {
                const auto content_length = std::stoull(resp.headers.value("content-length"));

                if (content_length > 0) {
                    resp.body = std::make_unique<netkit::body::stream_body>(*stream, content_length, std::move(overflow));
                }
            }

            return resp;
        }

        response request(method method, const std::string& path, const std::unique_ptr<body::basic_body>& body, const headers& headers = {}) {
            return request(get_method_string(method), path, body, headers);
        }

        response get(const std::string& path, const headers& headers = {}) {
            return request(method::GET, path, nullptr, headers);
        }

        response post(const std::string& path, const std::unique_ptr<body::basic_body>& body, const headers& headers = {}) {
            return request(method::POST, path, body, headers);
        }

        response put(const std::string& path, const std::unique_ptr<body::basic_body>& body, const headers& headers = {}) {
            return request(method::PUT, path, body, headers);
        }

        response patch(const std::string& path, const std::unique_ptr<body::basic_body>& body, const headers& headers = {}) {
            return request(method::PATCH, path, body, headers);
        }
    };
}
