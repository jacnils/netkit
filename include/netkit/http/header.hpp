#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <sstream>
#include <algorithm>
#include <charconv>

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

    [[nodiscard]] static headers parse_headers(const std::string& header_part) {
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

        const auto [ptr, ec] = std::from_chars(code_str.data(), code_str.data() + code_str.size(), code);

        if (ec != std::errc{} || ptr != code_str.data() + code_str.size())
            throw std::logic_error{"invalid HTTP status code"};

        return code;
    }
}
