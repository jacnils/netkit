# netkit

[![CMake on Linux, macOS, and Windows](https://github.com/jacnils/netkit/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/jacnils/netkit/actions/workflows/cmake-multi-platform.yml)

C++23 cross-platform networking toolkit library providing (a)sync socket and protocol abstractions

## Features

- OS-independent socket abstractions, both synchronous and asynchronous using C++20 coroutines
- Higher-level socket abstractions, including TCP, UDP and UDS
- HTTP/1.0 and HTTP/1.1 body parser, including headers and body.
- IPv4 and IPv6 support
- TCP and UDP support
- TLS/SSL sockets and HTTP abstraction (WolfSSL integration)
- Network interface enumeration
- Exceptions for errors
- Inheritable classes for easy extension
- Designed for C++23
- Support for Windows, Linux, macOS and other Unix-compatible systems.
- Support for DevkitPro (Wii and GameCube)
- Permissive MIT license, allowing use in both open source and proprietary software.

## Dependencies

- WolfSSL (optional)
- C++23 compiler
- CMake

## Options

netkit's CMakeLists.txt offers multiple options:

- NETKIT_ENABLE_WOLFSSL: Enable WolfSSL-backed SSL/TLS
- NETKIT_ENABLE_TESTS: Enable Catch2 tests for the main C++ library
- NETKIT_ENABLE_WINDOWS_CERTSTORE: Enable getting CA certificates from the Windows store
- NETKIT_ENABLE_FALLBACK_CA: Enable fallback hardcoded CA certificate (required for DevkitPro with TLS enabled)
- NETKIT_WOLFSSL_DEBUG: Enable debugging for WolfSSL and netkit WolfSSL functions
- NETKIT_BUILD_SHARED: Build a shared library version of netkit (If disabled, netkit will be static)

## Building

```bash
mkdir -p build/; cd build/
cmake .. -DCMAKE_BUILD_TYPE=Release # if using devkitpro, -DCMAKE_TOOLCHAIN_FILE=${DEVKITPRO:-/opt/devkitpro}/cmake/Wii.cmake
cmake --build .
cmake --install .
```

## Usage

You can use CMake and link with netkit, which will in turn link with the necessary libraries:

```cmake
...

find_package(netkit)

add_executable(
        MY_TARGET
        main.cpp
)
target_link_libraries(netkit-example PRIVATE
	netkit::netkit
)

...
```

See `examples/` for further examples of how to use the library.

netkit can also be statically linked, and for users of DevkitPro it will be automatically.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

Copyright (c) 2025-2026 Jacob Nilsson
