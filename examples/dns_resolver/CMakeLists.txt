cmake_minimum_required(VERSION 3.11)

project(dns_resolver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT TARGET netkit::netkit)
    find_package(netkit REQUIRED)
endif()

add_executable(dns_resolver main.cpp)

netkit_copy_dll(dns_resolver)

target_link_libraries(dns_resolver PRIVATE netkit::netkit)