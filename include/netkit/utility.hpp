/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file utility.hpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Provides various utility functions and classes used throughout the Netkit library.
 */
#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <sstream>
#include <cstdint>

#include <netkit/export.hpp>

/**
 * @brief Namespace for utility functions and classes.
 * @note Contains helper functions and classes that are not directly related to networking.
 */
namespace netkit::utility {
    /**
     * @brief Splits a string into tokens based on a delimiter.
     * @param str The string to split.
     * @param delimiter The delimiter to use for splitting.
     * @return A vector of strings containing the tokens.
     */
    [[nodiscard]] NETKIT_API std::vector<std::string> split(const std::string& str, const std::string& delimiter);

    /**
     * @brief Joins a vector of strings into a single string with a delimiter.
     * @param tokens The vector of strings to join.
     * @param delimiter The delimiter to use for joining.
     * @return A single string containing the joined tokens.
     */
    [[nodiscard]] NETKIT_API std::string join(const std::vector<std::string>& tokens, const std::string& delimiter);

    /**
     * @brief Generates a random alphanumeric string of a given length.
     * @param length The length of the random string to generate.
     * @return A random alphanumeric string.
     */
    [[nodiscard]] NETKIT_API std::string generate_random_string(std::size_t length = 64);
    /**
     * @brief URL-encodes a string.
     * @param str The string to encode.
     * @return The URL-encoded string.
     */
    NETKIT_API std::string url_encode(const std::string& str);
    /**
     * @brief URL-decodes a string.
     * @param str The string to decode.
     * @return The URL-decoded string.
     */
    NETKIT_API std::string url_decode(const std::string& str);
    /**
     * @brief Parses fields from an HTTP body as sent from, for example, a <form> element or a URL.
     * @param body The HTTP body to parse
     * @return Unordered map of fields that were parsed.
     */
    NETKIT_API std::unordered_map<std::string, std::string> parse_fields(const std::string& body);
    /**
     * @brief Converts Unix milliseconds to GMT format.
     * @param unix_millis Unix milliseconds (64-bit integer)
     * @return Formatted string
     */
    NETKIT_API std::string convert_unix_millis_to_gmt(int64_t unix_millis);
    /**
     * @brief Decodes chunked transfer encoding body
     * @param encoded HTTP body
     * @return Decoded body
     */
    [[nodiscard]] NETKIT_API std::string decode_chunked(const std::string& encoded);
    /**
     * @brief Returns the appropriate content type for a given file name.
     * @param fn The file name to check.
     * @return The content type as a string.
     */
    [[nodiscard]] NETKIT_API std::string get_appropriate_content_type(const std::string& fn);

    /**
     * @brief Reads the contents of a file into a string.
     * @param path The path to the file.
     */
    [[nodiscard]] NETKIT_API std::string read_file(const std::string& path);

    /**
     * @brief Writes a value of type T to an output stream
     * @param os Output stream to write to
     * @param val Constant value to write
     * @tparam T Any type which can be reinterpreted to const char*
     */
    template<typename T>
    void write(std::ostream& os, const T& val) {
        os.write(reinterpret_cast<const char*>(&val), sizeof(T));
    }
    /**
     * @brief Reads a value of type T to writable reference from an input stream
     * @note Any data parameter val holds will be discarded.
     * @tparam T Any type which is read- and writable and which can be cast to char*
     * @param is Input stream to read from
     * @param val Reference to a writable variable of type T
     */
    template<typename T>
    void read(std::istream& is, T& val) {
        is.read(reinterpret_cast<char*>(&val), sizeof(T));
    }
    /**
     * @brief Writes a string to an output stream
     * @param os Output stream to write to
     * @param str String to write
     */
    NETKIT_API void write_string(std::ostream& os, const std::string& str);
    /**
     * @brief Reads a string from an input stream
     * @param is Input stream to read from
     * @return String read from input stream is
     */
    NETKIT_API std::string read_string(std::istream& is);
    /**
     * @brief Returns the standard cache location used by netkit's internal DNS management
     * @note OS-specific, can be called on any system supported by netkit.
     * @note Do not attempt to guess the cache location, always use this function.
     * @return Returns the standard cache location in the form of an std::string
     */
    NETKIT_API std::string get_standard_cache_location();
    /**
     * @brief base64-encodes an input string
     * @param data Data to encode
     * @return base64-encoded string
     */
    NETKIT_API std::string encode_base64(const std::vector<uint8_t>& data);

    /**
     * @brief base64-decodes an input string_view
     * @param data Constant string_view reference containing the base64-encoded data to decode
     * @return std::vector<uint8_t> vector containing decoded data.
     */
    NETKIT_API std::vector<uint8_t> decode_base64(const std::string_view& data);
}