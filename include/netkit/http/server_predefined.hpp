/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file server_predefined.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides several predefined structs and enums used by the HTTP server components.
 */
#pragma once

#include <functional>
#include <unordered_map>
#include <string>
#include <cstdint>

namespace netkit::http::server {
    /**
     * @brief  Struct that represents a cookie.
     */
    struct cookie {
        std::string name{};
        std::string value{};
        int64_t expires{};
        std::string path{"/"};
        std::string domain{};
        std::string same_site{"Strict"};
        std::vector<std::string> attributes{};
        bool http_only{false};
        bool secure{false};
        std::unordered_map<std::string, std::string> extra_attributes{};
    };

    /**
     * @brief  Struct that represents an HTTP header.
     */
    struct header {
        std::string name{};
        std::string data{};
    };

    enum class redirect_type {
        permanent,
        temporary,
    };

    /**
     * @brief  Struct that contains the server settings.
     */
    struct server_settings {
        int port{8080};
        bool enable_session{true};
        std::string session_directory{"./"};
        std::string session_cookie_name{"session_id"};
        std::vector<std::string> associated_session_cookies{};
        int64_t max_request_size{1024 * 1024 * 1024};
        std::vector<std::string> blacklisted_ips{};
        bool trust_x_forwarded_for{false};
        int max_connections{-1};
        bool session_is_secure{true};
        std::function<std::unordered_map<std::string, std::string>(const std::string&)> read_from_session_file = nullptr;
        std::function<void(const std::string&, const std::unordered_map<std::string, std::string>&)> write_to_session_file = nullptr;
    };

    /**
     * @brief  Struct that contains the request data.
     */
    struct request {
        std::string endpoint{};
        std::unordered_map<std::string, std::string> query{};
        std::string content_type{};
        std::string body{};
        std::string raw_body{};
        std::string method{};
        std::string ip_address{};
        std::string user_agent{};
        unsigned int version{};
        std::vector<cookie> cookies{};
        std::unordered_map<std::string, std::string> session{};
        std::string session_id{};
        std::unordered_map<std::string, std::string> fields{};
    };

    /**
     * @brief  Struct that contains the response data.
     */
    struct response {
        int http_status{200};
        std::string body{};
        std::string content_type{"application/json"};
        std::string allow_origin{"*"};
        bool stop{false};
        std::vector<cookie> cookies{};
        std::vector<std::string> delete_cookies{};
        std::unordered_map<std::string, std::string> session{};
        std::string location{};
        redirect_type redirection{redirect_type::temporary};
        std::vector<header> headers{};
    };

    using request_callback = std::function<response(const request&)>;
}