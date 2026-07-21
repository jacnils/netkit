/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file main.c
 *  @license MIT
 *  @note Example code using the Netkit library.
 *  @note Only functional if Netkit was built with OpenSSL support.
 *  @note See examples/socket/main.c for a non-SSL/TLS version.
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

	std::cout << "Init...\n";

	try {
    netkit::sock::addr addr("google.com", 443, netkit::sock::addr_type::hostname);
    std::cout << "made addr\n";
    std::unique_ptr<netkit::sock::basic_sync_sock> _sock = std::make_unique<netkit::sock::sync_sock>(
        addr, netkit::sock::type::tcp);

    std::cout << "Made sock\n";

    netkit::sock::ssl_sync_sock sock((std::move(_sock)),
        netkit::sock::mode::client,
        netkit::sock::version::TLS_1_2,
        netkit::sock::verification::peer
        );

    std::cout << "Making request...\n";

    sock.connect();
    sock.perform_handshake();

    constexpr std::string_view request = "GET / HTTP/1.1\r\nHost: google.com\r\nConnection: close\r\n\r\n";
    std::string response;

    std::cout << "recv():";

    int sent = sock.send(request.data(), request.size());
	std::cout << "sent " << sent << " bytes\n";

    while (true) {
        auto res = sock.recv(6);
    
        response += res.data;
    
        if (res.status == netkit::sock::recv_status::closed)
	    break;

	if (res.status == netkit::sock::recv_status::timeout)
	    break;
    
        if (res.status == netkit::sock::recv_status::error) {
	    std::cout << "recv failed\n";
	    throw std::runtime_error("recv failed");
	}
    }

    std::cout << response << std::flush;

    std::cout << "Response not at all written to response.txt" << std::endl;
	} catch (std::exception& e) {
		std::cout << e.what() << "\n";
	}

	while (true) {};

	return EXIT_SUCCESS;
}
