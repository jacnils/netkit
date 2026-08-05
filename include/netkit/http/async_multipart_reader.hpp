/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file async_multipart_reader.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides the async_multipart_part struct for parsed multipart fields.
 */
#pragma once

#include <string>
#include <netkit/export.hpp>
#include <netkit/body/basic_async_body.hpp>
#include <netkit/http/multipart.hpp>
#include <netkit/http/multipart_reader_state.hpp>

namespace netkit::http::utility {
	class NETKIT_API async_multipart_reader {
	public:
		async_multipart_reader(const async_multipart_reader&) = delete;
		async_multipart_reader& operator=(const async_multipart_reader&) = delete;
		async_multipart_reader(async_multipart_reader&&) = delete;

		explicit async_multipart_reader(body::basic_async_body& body, std::string boundary) : source_(body), boundary_(std::move(boundary)) {};
		io::task<body::read_result> read_part(char* buffer, std::size_t max_bytes) noexcept;
		io::task<bool> next(async_multipart_part& part);
	private:
		friend class async_async_multipart_part_body;

		io::task<bool> fill_buffer();
		io::task<size_t> boundary_overlap() const;
		static void parse_part_headers(std::string_view headers, async_multipart_part& part);
		io::task<bool> read_boundary();
		io::task<bool> read_headers(async_multipart_part& part);

		body::basic_async_body& source_;

		std::string boundary_;
		std::string buffer_;

		multipart_state state_ = multipart_state::boundary;

		bool part_finished_ = false;
	};
}