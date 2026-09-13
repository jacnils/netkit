# netkit

[![CMake on Linux, macOS, and Windows](https://github.com/jacnils/netkit/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/jacnils/netkit/actions/workflows/cmake-multi-platform.yml)

[Docs](https://jacnils.github.io/netkit) [Wii HTTPS client showcase](https://www.youtube.com/watch?v=24BDbhny-eA)

C++23 cross-platform networking toolkit library providing (a)sync socket and protocol abstractions.

**Security notice: We advise against usage of this library for security or mission-critical use-cases, especially without a 
battle-tested reverse proxy like Apache or Nginx.**

Please read on to see design goals & rationale.

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

## Supported platforms

- Windows
- macOS
- Linux
- FreeBSD, OpenBSD, etc.*
- Nintendo GameCube/Nintendo Wii (through DevkitPPC/libogc)**

*Not tested regularly

**Coroutines are supported through threads; netkit:s built-in DNS resolver is not supported as of now.

## Dependencies

- C++23 compiler
- CMake
- WolfSSL
  - Automatically compiled and installed, unless -DNETKIT_ENABLE_WOLFSSL=OFF
  - Supported on all platforms netkit supports currently

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
# ...

find_package(netkit)

add_executable(
        MY_TARGET
        main.cpp
)
target_link_libraries(netkit-example PRIVATE
	netkit::netkit
)

# if using windows and dynamic/shared linking
# call netkit_copy_dll(TARGET) to copy libnetkit.dll and its potential dependencies to the build directory.
# if statically linking, or if not using windows, this is a nop.
netkit_copy_dll(netkit-example)

# ...
```

See `examples/` for further examples of how to use the library.

netkit can also be statically linked, and for users of DevkitPPC it will be automatically.

## Options

The CMakeLists.txt offers multiple options:

- NETKIT_ENABLE_WOLFSSL: Enable WolfSSL-backed SSL/TLS
- NETKIT_ENABLE_TESTS: Enable Catch2 tests for the main C++ library
- NETKIT_ENABLE_WINDOWS_CERTSTORE: Enable getting CA certificates from the Windows store
- NETKIT_ENABLE_FALLBACK_CA: Enable fallback hardcoded CA certificate (required for DevkitPro with TLS enabled)
- NETKIT_ENABLE_EPOLL: Enable epoll io_backend for Linux systems
- NETKIT_ENABLE_WSAPOLL: Enable WSAPoll io_backend for Windows systems
- NETKIT_ENABLE_KQUEUE: Enable kqueue io_backend for BSD systems and macOS
- NETKIT_ENABLE_HTTP: Enable HTTP abstractions in netkit
- NETKIT_ENABLE_DNS: Enable DNS abstractions in netkit
- NETKIT_WOLFSSL_DEBUG: Build WolfSSL with debugging features. Does nothing if used with NETKIT_USE_SYSTEM_WOLFSSL=ON
- NETKIT_BUILD_SHARED: Build netkit as a shared (dynamic) library. This will cause it not to be built statically
- NETKIT_BUILD_SHARED_WOLFSSL: Build WolfSSL as a shared (dynamic) library
- NETKIT_BUILD_MODULES: Build experimental C++20 modules that can be used instead of header files. These are merely wrappers, and their use is not recommended at this time.
- NETKIT_USE_SYSTEM_WOLFSSL: Use the system's installed WolfSSL instead of building it
- NETKIT_BUILD_EXAMPLES: Build netkit's examples
- NETKIT_DEBUG: Enable debugging features in netkit

## Design goals

There are tons of networking libraries for C++, but nothing really did what I wanted out of one. Our goals are as follows:

- Extensible clean codebase
- High level abstractions that don't have a steep learning curve, whilst not being in the way
- Adopting the latest C++ features early on, without too much of a thought about backwards compatibility
- No significant compromises on performance

Former goals which we have since shifted away from:

- Providing Unix-style sockets across all supported platforms.
  - This changed with 0.2.0, where we decided to design our own classes from scratch instead of writing basic wrappers around Unix sockets.
- Providing support for all backends one might use. As an example of this, OpenSSL support was dropped in 0.2.0.

## Common issues

- I get a stack overflow, what could be causing this?
  - Enable compiler optimizations, we recommend -O3

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

Copyright (c) 2025-2026 Jacob Nilsson
