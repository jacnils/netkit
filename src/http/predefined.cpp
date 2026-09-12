/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file predefined.cpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Implementation of predefined HTTP status codes and messages.
 */
#include <netkit/http/predefined.hpp>

#include <optional>
#include <string_view>
#include <sstream>

std::optional<std::string_view> netkit::http::get_message(int code) {
    for (const auto& status : status_list) {
        if (status.code == code) {
            return status.message;
        }
    }
    return std::nullopt;
}

const std::array<netkit::http::status_code, netkit::http::status_list.size()>& netkit::http::get_status_codes() {
    return status_list;
}

std::string netkit::http::get_method_string(netkit::http::method method) {
    std::stringstream ss;
    switch (method) {
    case method::method_get:
        ss << "GET"; break;
    case method::method_head:
        ss << "HEAD"; break;
    case method::method_post:
        ss << "POST"; break;
    case method::method_put:
        ss << "PUT"; break;
    case method::method_delete:
        ss << "DELETE"; break;
    case method::method_connect:
        ss << "CONNECT"; break;
    case method::method_options:
        ss << "OPTIONS"; break;
    case method::method_trace:
        ss << "TRACE"; break;
    case method::method_patch:
        ss << "PATCH"; break;
    case method::method_undefined:
        throw std::logic_error("undefined method");
    }
    return ss.str();
}