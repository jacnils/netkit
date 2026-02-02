cmake_minimum_required(VERSION 3.11)

project(netkit-example LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(netkit)

add_executable(
        netkit-example
        main.cpp
)
target_link_libraries(netkit-example PRIVATE
        netkit::netkit
)
