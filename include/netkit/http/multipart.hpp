/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file multipart.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides the multipart_part struct for parsed multipart fields.
 */
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <netkit/export.hpp>
#include <netkit/body/basic_body.hpp>
#include <netkit/body/basic_async_body.hpp>

namespace netkit::http::utility {
	struct NETKIT_API multipart_part {
		std::string name;
		std::string filename;
		std::string content_type;
		std::unique_ptr<body::basic_body> data;
	};

	struct NETKIT_API async_multipart_part {
		std::string name;
		std::string filename;
		std::string content_type;
		std::unique_ptr<body::basic_async_body> data;
	};

	NETKIT_API std::string extract_boundary(const std::string& content_type);
	NETKIT_API std::vector<multipart_part> parse_multipart_form_data(const char* body, std::size_t size, const std::string& content_type);
	NETKIT_API std::vector<multipart_part> parse_multipart_form_data(body::basic_body& body, const std::string& content_type);
}