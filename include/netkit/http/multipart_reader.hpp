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
#include <netkit/http/multipart_reader_state.hpp>

namespace netkit::http::utility {
	class NETKIT_API multipart_reader {
	public:
		multipart_reader(const multipart_reader&) = delete;
		multipart_reader& operator=(const multipart_reader&) = delete;
		multipart_reader(multipart_reader&&) = delete;
		
		explicit multipart_reader(body::basic_body& body, std::string boundary) : source_(body), boundary_(std::move(boundary)) {};
		body::read_result read_part(char* buffer, std::size_t max_bytes) noexcept;
		bool next(multipart_part& part);
	private:
		friend class multipart_part_body;

		bool fill_buffer();
		static void parse_part_headers(std::string_view headers, multipart_part& part);
		bool read_boundary();
		bool read_headers(multipart_part& part);

		body::basic_body& source_;

		std::string boundary_;
		std::string buffer_;

		multipart_state state_ = multipart_state::boundary;

		bool part_finished_ = false;
	};
}