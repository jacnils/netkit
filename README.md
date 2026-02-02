# netkit

[![CMake on Linux, macOS, and Windows](https://github.com/jacnils/netkit/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/jacnils/netkit/actions/workflows/cmake-multi-platform.yml)

C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.

## Features

- Binding, connecting, sending, receiving and closing synchronous TCP/UDP sockets
- HTTP/1.0 and HTTP/1.1 body parser, including headers and body.
- IPv4 and IPv6 support
- TCP and UDP support
- TLS/SSL sockets and HTTP abstraction (OpenSSL integration)
- DNS resolution
- Network interface enumeration
- Exceptions for errors
- Inheritable classes for easy extension
- Designed for C++23, with optional C bindings for use in non-C++ projects and interoperability with other programming languages entirely
- Support for Windows, Linux, macOS and other Unix-compatible systems.
- Permissive MIT license, allowing use in both open source and proprietary software.

Still missing:

- Asynchronous I/O
- Schannel support for Windows
- WebSocket abstraction

## Dependencies

- OpenSSL
- C++23 compiler
- CMake

## Building

```bash
mkdir -p build/; cd build/
cmake .. -DCMAKE_BUILD_TYPE=Release
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

If you wish to use the C bindings, link with `netkit::netkit-c` instead.

See `examples/c/` and `examples/cpp/` for further examples of how to use the library.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

Copyright (c) 2025-2026 Jacob Nilsson
