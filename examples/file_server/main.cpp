/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file main.cpp
 *  @license MIT
 *  @note Example code using the Netkit library.
 *  @brief A simple HTTP file server that lists files in a directory and serves them over HTTP.
 */
#include <iostream>
#include <filesystem>
#include <netkit/netkit.hpp>

enum class EntryType {
    File,
    Directory,
    Symlink,
};

struct Entry {
    std::string name{};
    std::string full_path{};
    EntryType type{EntryType::File};
};

std::vector<Entry> get_entries_in_directory(const std::string& directory) {
    std::filesystem::path dir{directory};
    std::vector<Entry> entries;
    if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)) {
        std::cerr << "Directory does not exist or is not a directory: " << directory << std::endl;
        return entries;
    }
    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        Entry e;
        e.name = entry.path().filename().string();
        e.full_path = entry.path().string();
        if (std::filesystem::is_regular_file(entry)) {
            e.type = EntryType::File;
        } else if (std::filesystem::is_directory(entry)) {
            e.type = EntryType::Directory;
        } else if (std::filesystem::is_symlink(entry)) {
            e.type = EntryType::Symlink;
        }
        entries.push_back(e);
    }
    return entries;
}

std::string root_directory{"/"};
std::string current_directory{"/"};

int main() {
    netkit::http::server::sync_server server(
        netkit::http::server::server_settings{
            .port = 1337,
            .enable_session = false,
            .session_directory = "./sessions",
            .session_cookie_name = "netkit-test",
            .trust_x_forwarded_for = false,
        },
        [](const netkit::http::server::request& request) -> netkit::http::server::response {
            netkit::http::server::response response;
            response.http_status = 200;
            response.content_type = "text/html";

            std::string body = "<html><body>";
            body += "<h1>Directory Listing</h1>";
            body += "<ul>";

            if (request.endpoint != "/") {
                body += "<li><a href=\"/$previous\">..</a></li>";
            } else {
                current_directory = root_directory;
            }

            if (request.endpoint == "/$previous") {
                // Go up one directory
                if (current_directory != root_directory) {
                    current_directory = current_directory.substr(0, current_directory.find_last_of('/'));
                    if (current_directory.empty()) {
                        current_directory = root_directory;
                    }
                }
            } else if (request.endpoint != "/" && request.endpoint.at(0) == '/') {
                auto fod = root_directory + request.endpoint.substr(1);
                fod = netkit::utility::url_decode(fod);

                if (std::filesystem::is_directory(fod)) {
                    if ((std::filesystem::status(fod).permissions() &
                         std::filesystem::perms::owner_read) != std::filesystem::perms::none)
                    {
                        current_directory = fod;
                    }
                } else {
                    // serve the file
                    std::filesystem::path file_path{fod};
                    if (std::filesystem::exists(file_path) && std::filesystem::is_regular_file(file_path)) {
                        response.body = netkit::utility::read_file(file_path.string());
                        response.content_type = netkit::utility::get_appropriate_content_type(file_path.filename().string());
                        return response;
                    } else {
                        response.http_status = 404;
                        response.body = "<p>404 Not Found</p>";
                        return response;
                    }
                }
            }

            auto entries = get_entries_in_directory(current_directory);
            for (const auto& entry : entries) {
                body += "<li><a href=\"" + entry.full_path + "\">" + entry.name + "</a></li>";
            }

            body += "</ul></body></html>";
            response.body = body;

            return response;
        });

    server.run();
}
