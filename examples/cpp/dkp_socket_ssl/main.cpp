/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file main.cpp
 *  @license MIT
 *  @note Example code using the Netkit library.
 *  @note Only functional if Netkit was built with OpenSSL support.
 *  @note See examples/socket/main.cpp for a non-SSL/TLS version.
 *  @brief A lower-level example demonstrating the usage of sync_sock to make a simple HTTP request, with SSL/TLS.
 */
#include <iostream>
#include <fstream>
#include <string_view>
#include <netkit/netkit.hpp>
#include <ogc/system.h>
#include <gccore.h>

int main() {
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

    netkit::sock::addr addr("www.google.com", 443, netkit::sock::addr_type::hostname);
    std::unique_ptr<netkit::sock::basic_sync_sock> _sock = std::make_unique<netkit::sock::sync_sock>(
        addr, netkit::sock::type::tcp);

    netkit::sock::ssl_sync_sock sock((std::move(_sock)),
        netkit::sock::mode::client,
        netkit::sock::version::TLS_1_2,
        netkit::sock::verification::peer
        );

    sock.connect();
    sock.perform_handshake();

    constexpr std::string_view request = "GET / HTTP/1.1\r\nHost: www.google.com\r\nConnection: close\r\n\r\n";
    std::string response;

    int sent = sock.send(request.data(), request.size());

    while (true) {
        auto res = sock.recv(6);
    
        response += res.data;
    
        if (res.status == netkit::sock::recv_status::closed)
			break;

		if (res.status == netkit::sock::recv_status::timeout)
			break;
    
        if (res.status == netkit::sock::recv_status::error)
			throw std::runtime_error("recv failed");
    }

    std::cout << response << std::flush;

	while (true) {};

	return EXIT_SUCCESS;
}
