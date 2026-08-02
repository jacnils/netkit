/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file main.cpp
 *  @license MIT
 *  @note Example code using the Netkit library.
 *  @note See examples/socket_ssl for a TLS/SSL version of this example.
 */
#include <iostream>
#include <fstream>
#include <string_view>
#include <netkit/tcp/tcp_stream.hpp>
#include <netkit/stream/tls_stream.hpp>
#ifdef NETKIT_DKP
#include <ogc/system.h>
#include <gccore.h>
#endif

int main() {
#ifdef NETKIT_DKP
	VIDEO_Init();
	WII_Initialize();

	const auto rmode = VIDEO_GetPreferredMode(nullptr);
	const auto xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

	console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);

	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();

	if (rmode->viTVMode&VI_NON_INTERLACE) {
		VIDEO_WaitVSync();
	}
#endif

	netkit::sock::addr addr{"google.com", 443, netkit::sock::addr_type::hostname};
	std::unique_ptr<netkit::tcp::tcp_stream> connector_ = std::make_unique<netkit::tcp::tcp_stream>(addr);

	connector_->connect();

	netkit::stream::tls_stream connector(std::move(connector_),
		netkit::stream::version::TLS_1_1,
		netkit::stream::verification::none
		);

	connector.perform_handshake();

    constexpr std::string_view request = "GET / HTTP/1.1\r\nHost: google.com\r\nConnection: close\r\n\r\n";

    auto write_result = connector.write_all(request);

	if (write_result.status != netkit::stream::stream_status::success) {
		throw std::runtime_error{"write failed"};
	}

	auto response = connector.read_all_string();

	connector.close();

#ifndef NETKIT_DKP
    std::ofstream file("response.txt");

    if (file.is_open()) {
    	file.write(response.data(), response.size());
        file.close();
    } else {
        std::cerr << "Failed to open file" << std::endl;
    }

    std::cout << "Response written to response.txt" << std::endl;
#else
	std::cout << response << std::endl;
	while (true) {}
#endif
}