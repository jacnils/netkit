cmake_minimum_required(VERSION 3.11)

project(c_http_server LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT TARGET netkit::netkit)
    find_package(netkit REQUIRED)
endif()

add_executable(c_http_server main.c)

netkit_copy_dll(c_http_server)

target_link_libraries(c_http_server PRIVATE netkit::netkit)