/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file predefined.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides several predefined HTTP enums, structs, and functions used throughout other HTTP components.
 */
#pragma once

#include <optional>
#include <array>
#include <string>
#include <string_view>

#include <netkit/export.hpp>

namespace netkit::http {
    /**
     * @brief HTTP version.
     */
    enum class version {
        HTTP_1_0,
        HTTP_1_1,
    };
    /**
     * @brief HTTP methods.
     */
    enum class method {
        GET,
        POST,
    };

    /**
     * @brief HTTP status codes and their messages.
     * @note This is a list of common HTTP status codes and their messages.
     */
    struct NETKIT_API status_code {
        int code;
        std::string_view message;
    };

    /**
     * @brief List of HTTP status codes and their messages.
     * @note This is a static array of http_status_code structs.
     */
    static constexpr std::array<status_code, 63> status_list = {{
        {100, "Continue"},
        {101, "Switching Protocols"},
        {102, "Processing"},
        {103, "Early Hints"},
        {200, "OK"},
        {201, "Created"},
        {202, "Accepted"},
        {203, "Non-Authoritative Information"},
        {204, "No Content"},
        {205, "Reset Content"},
        {206, "Partial Content"},
        {207, "Multi-Status"},
        {208, "Already Reported"},
        {226, "IM Used"},
        {300, "Multiple Choices"},
        {301, "Moved Permanently"},
        {302, "Found"},
        {303, "See Other"},
        {304, "Not Modified"},
        {305, "Use Proxy"},
        {306, "Switch Proxy"},
        {307, "Temporary Redirect"},
        {308, "Permanent Redirect"},
        {400, "Bad Request"},
        {401, "Unauthorized"},
        {402, "Payment Required"},
        {403, "Forbidden"},
        {404, "Not Found"},
        {405, "Method Not Allowed"},
        {406, "Not Acceptable"},
        {407, "Proxy Authentication Required"},
        {408, "Request Timeout"},
        {409, "Conflict"},
        {410, "Gone"},
        {411, "Length Required"},
        {412, "Precondition Failed"},
        {413, "Content Too Large"},
        {414, "URI Too Long"},
        {415, "Unsupported Media Type"},
        {416, "Range Not Satisfiable"},
        {417, "Expectation Failed"},
        {418, "I'm a teapot"},
        {421, "Misdirected Request"},
        {422, "Unprocessable Content"},
        {423, "Locked"},
        {424, "Failed Dependency"},
        {425, "Too Early"},
        {426, "Upgrade Required"},
        {428, "Precondition Required"},
        {429, "Too Many Requests"},
        {431, "Request Header Fields Too Large"},
        {451, "Unavailable For Legal Reasons"},
        {500, "Internal Server Error"},
        {501, "Not Implemented"},
        {502, "Bad Gateway"},
        {503, "Service Unavailable"},
        {504, "Gateway Timeout"},
        {505, "HTTP Version Not Supported"},
        {506, "Variant Also Negotiates"},
        {507, "Insufficient Storage"},
        {508, "Loop Detected"},
        {510, "Not Extended"},
        {511, "Network Authentication Required"}
    }};

    /**
     * @brief Get the HTTP message for a given status code.
     * @param code The HTTP status code.
     * @return An optional string_view containing the message, or std::nullopt if the code is not found.
     */
    NETKIT_API std::optional<std::string_view> get_message(int code);
    /**
     * @brief Get the list of HTTP status codes.
     * @return A constant reference to the array of HTTP status codes.
     */
    NETKIT_API const std::array<status_code, status_list.size()>& get_status_codes();
    /**
     * @brief A struct that represents an HTTP status line.
     * @note Used to parse the status line of an HTTP response.
     */
    struct NETKIT_API status_line {
        bool is_response{false};
        int status_code{-1};
        std::string method;
        std::string path;
        std::string version;
    };
}