/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file netkit-http.cppm
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Module partition wrapping netkit's http:: headers.
 */
module;

#include <netkit/definitions.hpp>
#include <netkit/http/basic_request_handler.hpp>
#include <netkit/http/basic_sync_server.hpp>
#include <netkit/http/multipart.hpp>
#include <netkit/http/multipart_reader.hpp>
#include <netkit/http/multipart_reader_state.hpp>
#include <netkit/http/async_multipart_reader.hpp>
#include <netkit/http/predefined.hpp>
#include <netkit/http/request_handler.hpp>
#include <netkit/http/server_predefined.hpp>
#include <netkit/http/sync_server.hpp>
#include <netkit/http/basic_async_server.hpp>
#include <netkit/http/async_server.hpp>
#include <netkit/http/client.hpp>
#include <netkit/http/header.hpp>
#include <netkit/http/async_client.hpp>

export module netkit:http;

export namespace netkit::http {
    using namespace ::netkit::http;
}

export namespace netkit::http::utility {
    using namespace ::netkit::http::utility;
}