cmake_minimum_required(VERSION 3.11)

project(c_tcp LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT TARGET netkit::netkit)
    find_package(netkit REQUIRED)
endif()

add_executable(c_tcp main.c)

netkit_copy_dll(c_tcp)

target_link_libraries(c_tcp PRIVATE netkit::netkit)