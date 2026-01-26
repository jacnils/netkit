/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file sync_client.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides a synchronous HTTP client class.
 */
#pragma once

#include <netkit/sock/sync_sock.hpp>
#include <netkit/http/predefined.hpp>
#include <netkit/http/body_parser.hpp>

#include <variant>

namespace netkit::sock {
    class ssl_sync_sock;
}

namespace netkit::http::client {
    /**
     * @brief A class that represents an HTTP client.
     */
    class NETKIT_API sync_client {
        std::string hostname{};
        std::string path{};
        int port{};
        method m{};
        version v{};

        std::string method_str{};
        std::string version_str{};
        std::vector<std::pair<std::string,std::string>> headers{};
        std::string body{};
        int timeout{5};

#ifdef NETKIT_OPENSSL
        using variant_sock = std::variant<netkit::sock::sync_sock, netkit::sock::ssl_sync_sock>;
#else
        using variant_sock = std::variant<network::sock::sync_sock>;
#endif
        [[nodiscard]] std::string make_request(const std::string& request) const;
    public:
        /**
         * @brief Constructs a client object.
         * @param hostname The hostname to connect to.
         * @param path The path to request.
         * @param port The port to use (default is 80).
         * @param m The HTTP method (GET or POST).
         * @param v The HTTP version (default is HTTP/1.1).
         * @param timeout The timeout in seconds (default is -1, which means no timeout).
         */
        sync_client(const std::string& hostname, const std::string& path, int port, method m, version v = version::HTTP_1_1, int timeout = -1);
        /**
         * @brief Append headers to the request.
         * @param in_headers The headers to append.
         */
        void append_headers(const std::vector<std::pair<std::string, std::string>>& in_headers);
        /**
         * @brief Set the request body.
         * @param in_body The body to set.
         */
        void set_body(const std::string& in_body);
        /**
         * @brief Set a header.
         * @param key The header key.
         * @param value The header value.
         */
        void set_header(const std::string& key, const std::string& value);
        /**
         * @brief Set the User-Agent header
         * @param user_agent The User-Agent string to set.
         */
        void set_user_agent(const std::string& user_agent);
        /**
         * @brief Set the Content-Type header.
         * @param content_type The Content-Type string to set.
         */
        void set_content_type(const std::string& content_type);
        /**
         * @brief Set the Accept header.
         * @param accept The Accept string to set.
         * @note Example: "application/json, text/html"
         */
        void set_accept(const std::string& accept);
        /**
         * @brief Set the Accept-Encoding header.
         * @param accept_encoding The Accept-Encoding string to set.
         * @note Example: "gzip, deflate"
         */
        void set_accept_encoding(const std::string& accept_encoding);
        /**
         * @brief Set the Accept-Language header.
         * @param accept_language The Accept-Language string to set.
         * @note Example: "en-US,en;q=0.5"
         */
        void set_accept_language(const std::string& accept_language);
        /**
         * @brief Set the Connection header.
         * @param connection The Connection string to set.
         * @note Example: "keep-alive"
         * @note Illegal in HTTP/2, which is not supported by this library.
         */
        void set_connection(const std::string& connection);
        /**
         * @brief Set the Referer header.
         * @param referer The Referer string to set.
         * @note Example: "https://example.com"
         */
        void set_referer(const std::string& referer);
        /**
         * @brief Set the Cache-Control header.
         * @param cache_control The Cache-Control string to set.
         * @note Example: "no-cache"
         */
        void set_cache_control(const std::string& cache_control);
        /**
         * @brief Set the Cookie header.
         * @param cookie The Cookie string to set.
         * @note Example: "sessionid=1234567890"
         */
        void set_cookie(const std::string& cookie);
        /**
         * @brief Set the Connect-Timeout header.
         * @param in_timeout The Connect-Timeout value to set.
         * @note Example: 5
         */
        void set_connect_timeout(int in_timeout);
        /**
         * @brief Get the request headers.
         * @return The request headers as a vector of key-value pairs.
         */
        [[nodiscard]] std::vector<std::pair<std::string, std::string>> get_headers() const;
        /**
         * @brief Get the request body.
         * @return The request body as a string.
         */
        [[nodiscard]] std::string get_body() const;
        /**
         * @brief Get the request hostname.
         * @return The request hostname as a string.
         */
        [[nodiscard]] std::string get_hostname() const;
        /**
         * @brief Get the request path.
         * @return The request path as a string.
         */
        [[nodiscard]] std::string get_path() const;
        /**
         * @brief Get the request port.
         * @return The request port as an integer.
         */
        [[nodiscard]] int get_port() const;
        /**
         * @brief Get the request method.
         * @return The request method as a method enum.
         */
        [[nodiscard]] method get_method() const;
        /**
         * @brief Get the request version.
         * @return The request version as a version enum.
         */
        [[nodiscard]] version get_version() const;
        /**
         * @brief Get the response from the server.
         * @return response object, parsed.
         */
        template <typename BP = body_parser<std::string>>
        [[nodiscard]] response get() const {
            std::string in_body{};
            in_body += this->method_str + " " + this->path + " " + this->version_str + "\r\n";

            for (const auto& [key, value] : this->headers) {
                in_body += key + ": " += value + "\r\n";
            }

            in_body += "Host: " + this->hostname + "\r\n";

            if (m == method::POST && !this->body.empty()) {
                in_body += "Content-Length: " + std::to_string(this->body.size()) + "\r\n";
                in_body += "\r\n" + this->body;
            } else {
                in_body += "\r\n";
            }

            auto ret = this->make_request(in_body);

            return BP(ret).parse();
        }
    };

}
