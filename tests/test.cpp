#include <iostream>
#include <fstream>
#include <string_view>
#include <netkit/netkit.hpp>
#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Ensure addr works", "[sock_addr]") {
	netkit::sock::addr addr("google.com", 443, netkit::sock::addr_type::hostname);
	REQUIRE(addr.get_hostname() == "google.com");
	REQUIRE(addr.get_ip().empty() == false);
	REQUIRE(addr.get_port() == 443);
	REQUIRE(addr.is_file_path() == false);
}