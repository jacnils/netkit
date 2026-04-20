#include <iostream>
#include <fstream>
#include <string_view>
#include <netkit/netkit.hpp>

int main() {
    netkit::sock::addr addr("/tmp/test.sock");
    netkit::sock::sync_sock sock(addr, netkit::sock::type::unix);

    sock.bind();
    sock.listen();
    
    while (true) {
    	auto rec = sock.accept();
	auto buffer = rec->recv().data;
	std::cout << buffer << "\n";
    }

    return 0;
}
