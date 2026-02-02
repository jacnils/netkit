cmake_minimum_required(VERSION 3.11)

project(netkit-example LANGUAGES C)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(netkit REQUIRED)

add_executable(
        netkit-example
        main.c
)
target_link_libraries(netkit-example PRIVATE netkit::netkit-c)
