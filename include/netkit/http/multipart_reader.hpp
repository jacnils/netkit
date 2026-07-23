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
#include <netkit/export.hpp>
#include <netkit/body/basic_body.hpp>
#include <netkit/http/multipart.hpp>

namespace netkit::http::utility {
	enum class NETKIT_API multipart_state {
		boundary,
		headers,
		data,
		finished
	};

	class NETKIT_API multipart_reader {
	public:
		explicit multipart_reader(body::basic_body& body, std::string boundary) : source_(body), boundary_(std::move(boundary)) {};
		body::read_result read_part(char* buffer, std::size_t max_bytes) noexcept;
		bool next(multipart_part& part);
	private:
		friend class multipart_part_body;

		bool fill_buffer();
		void parse_part_headers(std::string_view headers, multipart_part& part);
		bool read_boundary();
		bool read_headers(multipart_part& part);
		body::read_result read_current_part(char* out, size_t max) noexcept;

		body::basic_body& source_;

		std::string boundary_;
		std::string buffer_;

		multipart_state state_ = multipart_state::boundary;

		bool part_finished_ = false;
	};
}