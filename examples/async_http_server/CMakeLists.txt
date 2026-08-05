cmake_minimum_required(VERSION 3.11)

project(async_http_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT TARGET netkit::netkit)
    find_package(netkit REQUIRED)
endif()

add_executable(async_http_server main.cpp)

target_link_libraries(async_http_server PRIVATE netkit::netkit)
