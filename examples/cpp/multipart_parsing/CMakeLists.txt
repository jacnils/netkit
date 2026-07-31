cmake_minimum_required(VERSION 3.11)

project(multipart_parsing LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT TARGET netkit::netkit)
    find_package(netkit REQUIRED)
endif()

add_executable(multipart_parsing main.cpp)

target_link_libraries(multipart_parsing PRIVATE netkit::netkit)