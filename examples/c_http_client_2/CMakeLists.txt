cmake_minimum_required(VERSION 3.11)

project(c_http_client_2 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT TARGET netkit::netkit)
    find_package(netkit REQUIRED)
endif()

add_executable(c_http_client_2 main.c)

netkit_copy_dll(c_http_client_2)

target_link_libraries(c_http_client_2 PRIVATE netkit::netkit)