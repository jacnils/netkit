#include <iostream>
#include <fstream>
#include <netkit/netkit.hpp>
#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <set>
#include <unordered_set>

TEST_CASE("Ensure addr works", "[sock_addr]") {
	netkit::sock::addr addr("google.com", 443, netkit::sock::addr_type::hostname);
	REQUIRE(addr.get_hostname() == "google.com");
	REQUIRE(addr.get_ip().empty() == false);
	REQUIRE(addr.get_port() == 443);
	REQUIRE(addr.is_file_path() == false);

	netkit::sock::addr addr2("/tmp/socket");
	REQUIRE(addr2.get_path().string() == "/tmp/socket");
	REQUIRE(addr2.is_file_path() == true);
}

TEST_CASE("Ensure network utility functions work", "[network_utility]") {
	std::string valid_v4 = "1.0.1.0";
	std::string invalid_v4 = "999.0.0.1";
	std::string valid_v6 = "2001:0db8:85a3:0000:0000:8a2e:0370:7334";
	std::string invalid_v6 = "2001:0db8:85a3:0000:0000:8a2e:0370:zzzz";
	REQUIRE(netkit::network::is_ipv4(valid_v4) == true);
	REQUIRE(netkit::network::is_ipv4(invalid_v4) == false);
	REQUIRE(netkit::network::is_ipv6(valid_v6) == true);
	REQUIRE(netkit::network::is_ipv6(invalid_v6) == false);
	REQUIRE(netkit::network::is_valid_port(80) == true);
	REQUIRE(netkit::network::is_valid_port(70000) == false);
}
TEST_CASE("Ensure general utility functions work", "[utility]") {
	REQUIRE(netkit::utility::convert_unix_millis_to_gmt(1700000000000) == "Tue, 14 Nov 2023 22:13:20 GMT");
	REQUIRE(netkit::utility::generate_random_string().length() == 64);
	REQUIRE(netkit::utility::generate_random_string(256).length() == 256);

	std::set<std::string> generated_strings;
	for (int i = 0; i < 10000; i++) {
		std::string rand_str = netkit::utility::generate_random_string(32);
		REQUIRE(generated_strings.find(rand_str) == generated_strings.end());
		generated_strings.insert(rand_str);
	}

	std::unordered_map<std::string, std::string> fields = netkit::utility::parse_fields("key1=value1&key2=value2&key3=value3");
	REQUIRE(fields.size() == 3);
	REQUIRE(fields["key1"] == "value1");
	REQUIRE(fields["key2"] == "value2");
	REQUIRE(fields["key3"] == "value3");

	std::unordered_map<std::string, std::string> fields2 = netkit::utility::parse_fields("key1=value1&key2=&=value3&key4=value4&&key5=value5");
	REQUIRE(fields2.size() == 5);
	REQUIRE(fields2["key1"] == "value1");
	REQUIRE(fields2["key2"].empty());
	REQUIRE(fields2["key4"] == "value4");
	REQUIRE(fields2["key5"] == "value5");

	std::unordered_map<std::string, std::string> content_types = {
		{".html", "text/html"},
		{".css", "text/css"},
		{".js", "text/javascript"},
		{".png", "image/png"},
		{".jpg", "image/jpeg"},
		{".jpeg", "image/jpeg"},
		{".gif", "image/gif"},
		{".txt", "text/plain"},
		{".json", "application/json"},
		{".xml", "application/xml"},
		{".pdf", "application/pdf"},
	};
	for (const auto& [ext, expected_type] : content_types) {
		REQUIRE(netkit::utility::get_appropriate_content_type(ext) == expected_type);
	}

	REQUIRE(netkit::utility::url_encode("hello world!") == "hello%20world%21");
	REQUIRE(netkit::utility::url_decode("hello%20world%21") == "hello world!");

	std::string original = "This is a test! 1234 ~`!@#$%^&*()_+-={}|[]\\:\";'<>?,./";
	std::string encoded = netkit::utility::url_encode(original);
	std::string decoded = netkit::utility::url_decode(encoded);

	REQUIRE(decoded == original);

	std::string encoded_encoded = netkit::utility::url_encode(encoded);
	REQUIRE(encoded_encoded != encoded);
	REQUIRE(netkit::utility::url_decode(encoded_encoded) == encoded);
	REQUIRE(netkit::utility::url_decode(netkit::utility::url_decode(encoded_encoded)) == original);

	auto output = netkit::utility::get_standard_cache_location();
#ifdef NETKIT_WINDOWS
	REQUIRE(output.find(std::getenv("LOCALAPPDATA")) != std::string::npos);
#elif defined(NETKIT_MACOS)
	REQUIRE(output.find("Library/Caches") != std::string::npos);
#else
	REQUIRE(output.find(".cache") != std::string::npos);
#endif

	std::string chunked_http = "4\r\nWiki\r\n5\r\npedia\r\nE\r\n in\r\n\r\nchunks.\r\n0\r\n\r\n";

	REQUIRE(netkit::utility::decode_chunked(chunked_http) == "Wikipedia in\r\n\r\nchunks.");

	std::stringstream ss;
	netkit::utility::write_string(ss, "Hello, World!");
	REQUIRE(netkit::utility::read_string(ss) == "Hello, World!");

	ss = {};
	int32_t int_val = 123456789;
	netkit::utility::write(ss, int_val);
	ss.seekg(0);
	int32_t int_val_read;
	netkit::utility::read(ss, int_val_read);
	REQUIRE(int_val == int_val_read);

	std::vector<uint8_t> data = {'A', 'B', 'C', 'D', 'E', 'F'};
	std::string b64_encoded = netkit::utility::encode_base64(data);
	REQUIRE(b64_encoded == "QUJDREVG");

	std::vector<uint8_t> b64_decoded = netkit::utility::decode_base64(b64_encoded);
	REQUIRE(b64_decoded == data);

	std::string to_split = "one,two,three,four,five";
	auto tokens = netkit::utility::split(to_split, ",");
	REQUIRE(tokens.size() == 5);
	REQUIRE(tokens[0] == "one");
	REQUIRE(tokens[1] == "two");
	REQUIRE(tokens[2] == "three");
	REQUIRE(tokens[3] == "four");
	REQUIRE(tokens[4] == "five");
	std::string joined = netkit::utility::join(tokens, ",");
	REQUIRE(joined == to_split);
}