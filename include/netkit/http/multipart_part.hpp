/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file multipart_part.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides the multipart_part struct for parsed multipart fields.
 */
#pragma once

namespace netkit::http::server {
	struct multipart_part {
		std::string name;
		std::string filename;
		std::string content_type;
		std::string data;
	};
}