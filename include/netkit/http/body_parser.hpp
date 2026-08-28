/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file body_parser.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides a basic HTTP body parser class template.
 */
#pragma once

#include <netkit/http/predefined.hpp>
#include <netkit/http/response.hpp>
#include <netkit/except.hpp>

#include <vector>
#include <sstream>
#include <cstring>

namespace netkit::http {
    /**
     * @brief Basic HTTP body parser.
     * @note Splits the body into headers and body.
     */
    template <typename T = std::istringstream,
              typename R = client::response,
              typename VPS = std::vector<std::pair<T,T>>>
    class body_parser {
        const T& input;
        R ret{};
        VPS headers{};
        T body{};
        status_line status{};
    public:
        /**
         * @brief Constructs a basic_body_parser object.
         * @param input The body to parse.
         */
        explicit body_parser(const T& input) : input(input), ret({}) {
            constexpr auto HEADER_END = "\r\n\r\n";
            const auto pos = input.find(HEADER_END);
            if (pos == std::string::npos) {
                throw parsing_error("no header terminator; invalid HTTP body");
            }

            if (pos + strlen(HEADER_END) < input.length()) {
                this->body = input.substr(pos + strlen(HEADER_END));
            }

            std::string line{};
            std::istringstream hs(input.substr(0, pos));
            while (std::getline(hs, line)) {
                if (line.back() == '\r') line.pop_back();
                const auto cpos = line.find(':');

                if (cpos != std::string::npos) {
                    auto key = line.substr(0, cpos);
                    auto value = line.substr(cpos + 1);
                    auto trim = [](std::string& s) {
                        s.erase(0, s.find_first_not_of(" \t"));
                        s.erase(s.find_last_not_of(" \t") + 1);
                    };
                    trim(key);
                    trim(value);

                    this->headers.emplace_back(key, value);
                }
            }
        }
        ~body_parser() = default;
        /**
         * @brief Parse the status line from the input.
         * @return The parsed status_line object.
         */
        http::status_line get_status_line() {
            size_t newline_pos = input.find('\n');
            std::string line = (newline_pos != std::string::npos) ? input.substr(0, newline_pos) : input;

            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            if (line.empty()) {
                throw parsing_error("empty HTTP start line");
            }

            std::istringstream iss(line);

            http::status_line msg;

            if (line.compare(0, 5, "HTTP/") == 0) {
                msg.is_response = true;
                iss >> msg.version >> msg.status_code;
                if (iss.fail() || msg.status_code < 100 || msg.status_code > 599) {
                    throw parsing_error("invalid HTTP response status line");
                }
            } else {
                iss >> msg.method >> msg.path >> msg.version;
                if (iss.fail()) {
                    throw parsing_error("invalid HTTP request start line");
                }
                msg.is_response = false;
            }

            return msg;
        }
        /**
         * @brief Get the input stream.
         * @return The input stream (reference)
         */
        [[nodiscard]] T& get_input() {
            return this->input;
        }
        /**
         * @brief Get the body (excluding any headers)
         */
        [[nodiscard]] T& get_body() {
            return this->body;
        }
        /**
         * @brief Get the headers.
         * @return The headers (reference)
         */
        [[nodiscard]] VPS& get_headers() {
            return this->headers;
        }
        /**
         * @brief Parse the body.
         * @return The parsed response (reference)
         */
        [[nodiscard]] R& parse() {
            this->ret = R{};
            this->ret.status = this->get_status_line();
            this->ret.headers = this->get_headers();
            this->ret.body = this->get_body();
            this->ret.body = this->input;
            return this->ret;
        }
    };
}