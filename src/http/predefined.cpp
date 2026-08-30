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