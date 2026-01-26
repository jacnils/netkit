/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file utility.cpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Implementation of various utility functions used throughout the Netkit library.
 */
#include <netkit/definitions.hpp>
#include <netkit/utility.hpp>

#include <iterator>
#include <random>
#include <ostream>
#include <fstream>
#include <chrono>
#include <filesystem>

#ifdef NETKIT_WINDOWS
#include <ws2tcpip.h>
#include <iphlpapi.h>
#endif

namespace netkit::utility {
    [[nodiscard]] std::vector<std::string> split(const std::string& str, const std::string& delimiter) {
        std::vector<std::string> tokens;
        size_t start = 0;
        size_t end = str.find(delimiter);
        while (end != std::string::npos) {
            tokens.push_back(str.substr(start, end - start));
            start = end + delimiter.length();
            end = str.find(delimiter, start);
        }
        tokens.push_back(str.substr(start, end));
        return tokens;
    }
    [[nodiscard]] std::string join(const std::vector<std::string>& tokens, const std::string& delimiter) {
        if (tokens.empty()) return "";
        std::ostringstream oss;
        std::copy(tokens.begin(), tokens.end() - 1, std::ostream_iterator<std::string>(oss, delimiter.c_str()));
        oss << tokens.back();
        return oss.str();
    }
    [[nodiscard]] std::string generate_random_string(const std::size_t length) {
        static constexpr char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";

        static constexpr size_t charset_size = sizeof(charset) - 1;
        std::random_device rd;

        std::string result;
        result.reserve(length);

        for (std::size_t i = 0; i < length; ++i) {
            result += charset[rd() % charset_size];
        }

        return result;
    }
    std::string url_encode(const std::string& str) {
        std::string ret;
        for (int i = 0; i < str.length(); i++) {
            char ch = str[i];
            if (isalnum(ch) || ch == '-' || ch == '_' || ch == '.' || ch == '~') {
                ret += ch;
            } else if (ch == ' ') {
                ret += '+';
            } else {
                ret += '%';
                ret += "0123456789ABCDEF"[ch >> 4];
                ret += "0123456789ABCDEF"[ch & 15];
            }
        }
        return ret;
    }
    std::string url_decode(const std::string& str) {
        std::string result;
        result.reserve(str.size());

        for (std::size_t i = 0; i < str.length(); ++i) {
            if (str[i] == '%') {
                if (i + 2 < str.length()) {
                    char hex1 = str[i + 1];
                    char hex2 = str[i + 2];

                    if (std::isxdigit(hex1) && std::isxdigit(hex2)) {
                        int high = std::isdigit(hex1) ? hex1 - '0' : std::tolower(hex1) - 'a' + 10;
                        int low  = std::isdigit(hex2) ? hex2 - '0' : std::tolower(hex2) - 'a' + 10;
                        result += static_cast<char>((high << 4) | low);
                        i += 2;
                        continue;
                    }
                }
                result += '%';
            } else if (str[i] == '+') {
                result += ' ';
            } else {
                result += str[i];
            }
        }

        return result;
    }
    std::unordered_map<std::string, std::string> parse_fields(const std::string& body) {
        std::unordered_map<std::string, std::string> result;

        std::size_t start = 0;
        while (start < body.length()) {
            std::size_t end = body.find('&', start);
            if (end == std::string::npos) end = body.length();

            std::string pair = body.substr(start, end - start);
            std::size_t eq_pos = pair.find('=');

            if (eq_pos != std::string::npos) {
                std::string key = url_decode(pair.substr(0, eq_pos));
                std::string value = url_decode(pair.substr(eq_pos + 1));
                result[std::move(key)] = std::move(value);
            } else if (!pair.empty()) {
                std::string key = url_decode(pair);
                result[std::move(key)] = "";
            }

            start = end + 1;
        }

        return result;
    }
    std::string convert_unix_millis_to_gmt(const int64_t unix_millis) {
        if (unix_millis == -1) {
            return "Thu, 01 Jan 1970 00:00:00 GMT";
        }

        std::time_t time = unix_millis / 1000;
        std::tm* tm = std::gmtime(&time);
        char buffer[80];
        std::strftime(buffer, 80, "%a, %d %b %Y %H:%M:%S GMT", tm);
        return {(buffer)};
    }
    [[nodiscard]] std::string decode_chunked(const std::string& encoded) {
        std::string dec;
        size_t pos = 0;

        while (pos < encoded.size()) {
            size_t crlf = encoded.find("\r\n", pos);
            if (crlf == std::string::npos) break;

            std::string size_str = encoded.substr(pos, crlf - pos);
            size_t chunk_size = 0;
            try {
                chunk_size = std::stoul(size_str, nullptr, 16);
            } catch (...) {
                break;
            }

            if (chunk_size == 0) break;

            pos = crlf + 2;
            if (pos + chunk_size > encoded.size()) break;

            dec.append(encoded.substr(pos, chunk_size));
            pos += chunk_size + 2;
        }

        return dec;
    }
    [[nodiscard]] std::string get_appropriate_content_type(const std::string& fn) {
        std::size_t pos = fn.find_last_of('.');
        if (pos == std::string::npos) {
            return "application/octet-stream";
        }

        std::string file = fn.substr(pos);

        static const std::unordered_map<std::string, std::string> content_type_map {
            {".aac", "audio/aac"},
            {".abw", "application/x-abiword"},
            {".apng", "image/apng"},
            {".arc", "application/x-freearc"},
            {".avif", "image/avif"},
            {".avi", "video/x-msvideo"},
            {".azw", "application/vnd.amazon.ebook"},
            {".bin", "application/octet-stream"},
            {".bmp", "image/bmp"},
            {".bz", "application/x-bzip"},
            {".bz2", "application/x-bzip2"},
            {".cda", "application/x-cdf"},
            {".csh", "application/x-csh"},
            {".css", "text/css"},
            {".csv", "text/csv"},
            {".doc", "application/msword"},
            {".docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
            {".eot", "application/vnd.ms-fontobject"},
            {".epub", "application/epub+zip"},
            {".gz", "application/gzip"},
            {".gif", "image/gif"},
            {".htm", "text/html"},
            {".html", "text/html"},
            {".ico", "image/vnd.microsoft.icon"},
            {".ics", "text/calendar"},
            {".jar", "application/java-archive"},
            {".jpeg", "image/jpeg"},
            {".jpg", "image/jpeg"},
            {".js", "text/javascript"},
            {".json", "application/json"},
            {".jsonld", "application/ld+json"},
            {".mid", "audio/x-midi"},
            {".midi", "audio/midi"},
            {".mjs", "text/javascript"},
            {".mp3", "audio/mpeg"},
            {".mp4", "video/mp4"},
            {".flac", "audio/flac"},
            {".mpeg", "video/mpeg"},
            {".mpkg", "application/vnd.apple.installer+xml"},
            {".odp", "application/vnd.oasis.opendocument.presentation"},
            {".ods", "application/vnd.oasis.opendocument.spreadsheet"},
            {".odt", "application/vnd.oasis.opendocument.text"},
            {".oga", "audio/ogg"},
            {".ogv", "video/ogg"},
            {".ogx", "application/ogg"},
            {".opus", "audio/ogg"},
            {".otf", "font/otf"},
            {".png", "image/png"},
            {".pdf", "application/pdf"},
            {".php", "application/x-httpd-php"},
            {".ppt", "application/vnd.ms-powerpoint"},
            {".pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
            {".rar", "application/vnd.rar"},
            {".rtf", "application/rtf"},
            {".sh", "application/x-sh"},
            {".svg", "image/svg+xml"},
            {".tar", "application/x-tar"},
            {".tif", "image/tiff"},
            {".tiff", "image/tiff"},
            {".ts", "video/mp2t"},
            {".ttf", "font/ttf"},
            {".txt", "text/plain"},
            {".vsd", "application/vnd.visio"},
            {".wav", "audio/wav"},
            {".weba", "audio/webm"},
            {".webm", "video/webm"},
            {".webp", "image/webp"},
            {".woff", "font/woff"},
            {".woff2", "font/woff2"},
            {".xhtml", "application/xhtml+xml"},
            {".xls", "application/vnd.ms-excel"},
            {".xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
            {".xml", "application/xml"},
            {".xul", "application/vnd.mozilla.xul+xml"},
            {".zip", "application/zip"},
            {".3gp", "video/3gpp"},
            {".3g2", "video/3gpp2"},
            {".7z", "application/x-7z-compressed"},
        };

        if (content_type_map.contains(file)) {
            return content_type_map.at(file);
        } else {
            return "application/octet-stream";
        }
    }
    [[nodiscard]] std::string read_file(const std::string& path) {
        std::ifstream file(path, std::ios::in | std::ios::binary | std::ios::ate);
        if (!file) {
            throw std::runtime_error("failed to open file: " + path);
        }

        std::streamsize size = file.tellg();
        std::string buffer(size, '\0');

        file.seekg(0, std::ios::beg);
        if (!file.read(&buffer[0], size)) {
            throw std::runtime_error("failed to read file: " + path);
        }

        return buffer;
    }

    void write_string(std::ostream& os, const std::string& str) {
        auto len = static_cast<uint32_t>(str.size());
        write(os, len);
        os.write(str.data(), len);
    }

    std::string read_string(std::istream& is) {
        uint32_t len;
        read(is, len);
        std::string str(len, '\0');
        is.read(&str[0], len);
        return str;
    }

    std::string get_standard_cache_location() {
        const std::string cache_filename = "dns_cache";
        const std::string folder_name = "netkit";

        std::filesystem::path base_path;

#ifdef NETKIT_WINDOWS
    	char appdata[MAX_PATH];
    	DWORD len = GetEnvironmentVariableA("LOCALAPPDATA", appdata, sizeof(appdata));
    	if (len > 0) {
    		base_path = appdata;
    	} else {
    		base_path = std::filesystem::temp_directory_path();
    	}
    	base_path /= folder_name;
#elifdef NETKIT_MACOS
        if (const char* home = std::getenv("HOME")) {
            base_path = std::filesystem::path(home) / "Library" / "Caches" / folder_name;
        } else {
            base_path = std::filesystem::temp_directory_path();
        }

#elifdef NETKIT_UNIX
        const char* xdg = std::getenv("XDG_CACHE_HOME");
        if (xdg) {
            base_path = std::filesystem::path(xdg) / folder_name;
        } else if (const char* home = std::getenv("HOME")) {
            base_path = std::filesystem::path(home) / ".cache" / folder_name;
        } else {
            base_path = std::filesystem::temp_directory_path();  // fallback
        }
#else
#error "Unsupported platform for DNS cache location; write your own derivitive class"
#endif
        std::filesystem::create_directories(base_path);
        return (base_path / cache_filename).string();
    }

    std::string encode_base64(const std::vector<uint8_t>& data) {
    	static constexpr std::string_view base64_a = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

    	std::stringstream ret;
    	size_t len = data.size();
    	size_t i = 0;

    	while (i < len) {
    		uint32_t a = i < len ? static_cast<uint8_t>(data[i++]) : 0;
    		uint32_t b = i < len ? static_cast<uint8_t>(data[i++]) : 0;
    		uint32_t c = i < len ? static_cast<uint8_t>(data[i++]) : 0;

    		uint32_t triple = (a << 16) + (b << 8) + c;

    		ret << base64_a[(triple >> 18) & 0x3F]
					<< base64_a[(triple >> 12) & 0x3F]
					<< base64_a[(triple >> 6) & 0x3F]
					<< base64_a[triple & 0x3F];
    	}

		if (size_t pad = len % 3) {
    		ret.seekp(-static_cast<int>(3 - pad), std::ios_base::end);
    		for (size_t j = 0; j < 3 - pad; ++j) {
    			ret << '=';
    		}
    	}

    	return ret.str();
    }

    std::vector<uint8_t> decode_base64(const std::string_view& data) {
    	static constexpr std::string_view base64_a = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

    	std::vector<uint8_t> ret;
    	int three{};
    	uint32_t four{};

    	for (char c : data) {
    		if (std::isspace(c)) {
    			continue;
    		}

    		if ((c == '=') || (c == '\0')) {
    			break;
    		}

    		if (std::isalnum(c) || (c == '-') || (c == '_')) {
    			four = (four << 6) | base64_a.find(c);
    			three += 6;

    			if (three >= 8) {
    				three -= 8;
    				ret.push_back(static_cast<uint8_t>((four >> three) & 0xFF));
    			}
    		} else {
    			throw std::runtime_error{"invalid string"};
    		}
    	}

    	return ret;
    }
}