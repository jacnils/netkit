/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file response.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides a response struct for HTTP client responses.
 */
#pragma once

#include <netkit/http/predefined.hpp>

#include <string>
#include <vector>

namespace netkit::http::client {
  /**
   * @brief A struct that represents an HTTP response.
   */
    struct response {
        int status_code{};
        std::string body{};
        status_line status{};

        std::vector<std::pair<std::string, std::string>> headers{};
    };
}