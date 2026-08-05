cmake_minimum_required(VERSION 3.28)
project(netkit LANGUAGES CXX VERSION 0.4.0)

option(NETKIT_ENABLE_WOLFSSL "Enable WolfSSL support in netkit" ON)
option(NETKIT_ENABLE_TESTS "Enable building tests" OFF)
option(NETKIT_ENABLE_WINDOWS_CERTSTORE "Enable getting CA certificates from the Windows store" ON)
option(NETKIT_ENABLE_FALLBACK_CA "Enable fallback CA certficate (required for DevkitPro)" ON)
option(NETKIT_ENABLE_SOCK_CUSTOM_RESOLVER "Enable using netkit's DNS resolver instead of the system resolver" OFF)
option(NETKIT_ENABLE_EPOLL "Enable using epoll backend (Linux only)" ON)
option(NETKIT_ENABLE_WSAPOLL "Enable using wsapoll backend (Windows only)" ON)
option(NETKIT_ENABLE_HTTP "Enable netkit's HTTP abstractions" ON)
option(NETKIT_ENABLE_DNS "Enable netkit's DNS features" ON)
option(NETKIT_WOLFSSL_DEBUG "Enable WolfSSL debugging" OFF)
option(NETKIT_BUILD_SHARED "Build netkit as a shared library" ON)
option(NETKIT_USE_SYSTEM_WOLFSSL "Use system WolfSSL if found (not recommended)" OFF)
option(NETKIT_BUILD_EXAMPLES "Build netkit's examples" OFF)

if (CMAKE_SYSTEM_NAME STREQUAL "NintendoWii" OR CMAKE_SYSTEM_NAME STREQUAL "NintendoGameCube")
    set(NETKIT_DKP ON)
else()
    set(NETKIT_DKP OFF)
endif()

if (NETKIT_BUILD_SHARED)
    set(NETKIT_LIB_TYPE SHARED)
else()
    set(NETKIT_LIB_TYPE STATIC)
endif()

# we can only have static builds with devkitpro
if (NETKIT_DKP)
    set(NETKIT_LIB_TYPE STATIC)
endif()

# for gcc
if (NOT NETKIT_BUILD_SHARED AND WIN32)
    set(CMAKE_EXE_LINKER_FLAGS
            "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++"
    )
endif()

if (WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CTest)
if (NETKIT_ENABLE_TESTS)
    enable_testing()
endif()

add_library(netkit ${NETKIT_LIB_TYPE}
        src/utility.cpp
        src/network/utility.cpp
        src/network/local_address.cpp
        src/network/network_interface.cpp
        src/network/ip_list.cpp
        src/dns/record_type.cpp
        src/dns/nameserver_list.cpp
        src/socket/addr.cpp
        src/dns/response_parser.cpp
        src/dns/query_builder.cpp
        src/socket/native/native_sync_socket.cpp
        src/dns/cache.cpp
        src/http/predefined.cpp
        src/http/sync_client.cpp
        src/crypto/windows/certs.cpp
        src/body/stream_body.cpp
        src/http/multipart.cpp
        src/http/multipart_reader.cpp
        src/body/multipart_part_body.cpp
        src/io/io_awaitable.cpp
        src/io/linux/io_backend.cpp
        src/socket/native/native_async_socket.cpp
        src/socket/native/native_sync_listener.cpp
        src/socket/native/native_async_listener.cpp
        src/socket/native/peer_helper.cpp
        src/tcp/tcp_server.cpp
        src/tcp/tcp_stream.cpp
        src/stream/socket_stream.cpp
        src/stream/socket_stream.cpp
        src/stream/async_socket_stream.cpp
        src/tcp/async_tcp_stream.cpp
        src/tcp/async_tcp_server.cpp
        src/io/fallback/io_backend.cpp
        src/io/windows/io_backend.cpp
        src/udp/udp_datagram.cpp
        src/udp/async_udp_datagram.cpp
        src/stream/wolfssl/tls_stream.cpp
        src/uds/async_uds_server.cpp
        src/uds/async_uds_stream.cpp
        src/uds/uds_server.cpp
        src/uds/uds_stream.cpp
        src/body/async_stream_body.cpp
        src/body/async_multipart_part_body.cpp
        src/http/async_multipart_reader.cpp
        src/body/async_chunked_body.cpp
        src/body/chunked_body.cpp
)

add_library(netkit::netkit ALIAS netkit)

set_target_properties(netkit PROPERTIES
        EXPORT_NAME netkit
        POSITION_INDEPENDENT_CODE ON
        VERSION 0.1.0
        SOVERSION 1
)

if (WIN32 AND NETKIT_BUILD_SHARED)
    target_compile_definitions(netkit
            PRIVATE NETKIT_BUILD_DLL
    )
endif()

target_include_directories(netkit
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

if (NETKIT_ENABLE_WOLFSSL)
    find_package(wolfSSL QUIET)

    if (NOT NETKIT_BUILD_SHARED)
        set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    endif()

    if (NOT wolfSSL_FOUND OR NETKIT_DKP OR NETKIT_WOLFSSL_DEBUG OR NOT NETKIT_USE_SYSTEM_WOLFSSL)
        message(STATUS "wolfSSL not found. Building from source...")

        include(FetchContent)

        set(WOLFSSL_CRYPT_TESTS OFF CACHE BOOL "" FORCE)
        set(WOLFSSL_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(WOLFSSL_ALT_CERT_CHAINS ON CACHE BOOL "" FORCE) # im not sure why wolfssl is not built with this by default but it fixes handshake with fallback certs

        if (NETKIT_WOLFSSL_DEBUG)
            message(STATUS "Compiling WolfSSL with DEBUG_WOLFSSL")
            add_compile_definitions(DEBUG_WOLFSSL)
            set(DEBUG_WOLFSSL ON CACHE BOOL "" FORCE)
        endif()

        if (NETKIT_DKP)
            set(WOLFSSL_USER_SETTINGS ON CACHE BOOL "" FORCE)

            add_compile_definitions(
                    WOLFSSL_USER_SETTINGS
                    WOLFSSL_USER_SETTINGS_FILE="user_settings.h"
            )

            include_directories(${CMAKE_SOURCE_DIR}/include/netkit/stream/wolfssl)
        endif()

        FetchContent_Declare(
                wolfssl
                GIT_REPOSITORY https://github.com/wolfSSL/wolfssl.git
                GIT_TAG v5.9.2-stable
        )

        FetchContent_MakeAvailable(wolfssl)

        set(WOLFSSL_TARGET wolfssl)
    else()
        set(WOLFSSL_TARGET wolfssl)
    endif()

    target_compile_definitions(netkit PUBLIC NETKIT_WOLFSSL)
    target_link_libraries(netkit PUBLIC ${WOLFSSL_TARGET})

    message(STATUS "WolfSSL enabled in netkit")
endif()

if (WIN32 AND NETKIT_ENABLE_WINDOWS_CERTSTORE)
    target_compile_definitions(netkit PUBLIC NETKIT_ENABLE_WINDOWS_CERTSTORE)
endif()

if (NETKIT_ENABLE_FALLBACK_CA)
    target_compile_definitions(netkit PUBLIC NETKIT_ENABLE_FALLBACK_CA)
endif()

if (NETKIT_WOLFSSL_DEBUG)
    target_compile_definitions(netkit PUBLIC NETKIT_WOLFSSL_DEBUG)
endif()

if (NETKIT_ENABLE_WOLFSSL OR NETKIT_ENABLE_OPENSSL)
    target_compile_definitions(netkit PUBLIC NETKIT_SSL)
endif()

if (NETKIT_BUILD_SHARED)
    target_compile_definitions(netkit PUBLIC NETKIT_SHARED)
endif()

if (NETKIT_ENABLE_SOCK_CUSTOM_RESOLVER)
    target_compile_definitions(netkit PUBLIC NETKIT_ENABLE_SOCK_CUSTOM_RESOLVER)
endif()

if (NETKIT_ENABLE_HTTP)
    target_compile_definitions(netkit PUBLIC NETKIT_HTTP)
endif()

if (NETKIT_ENABLE_DNS)
    target_compile_definitions(netkit PUBLIC NETKIT_DNS)
endif()

if (NETKIT_ENABLE_EPOLL)
    target_compile_definitions(netkit PUBLIC NETKIT_EPOLL)
endif()

if (NETKIT_ENABLE_WSAPOLL)
    target_compile_definitions(netkit PUBLIC NETKIT_WSAPOLL)
endif()

if (WIN32)
    add_definitions(-DNOMINMAX)
endif()

if (WIN32)
    target_link_libraries(netkit PUBLIC ws2_32 iphlpapi dnsapi crypt32)
elseif (UNIX AND NOT APPLE)
    target_link_libraries(netkit PUBLIC resolv)
elseif (APPLE)
    target_link_libraries(netkit PUBLIC
            resolv
            "-framework SystemConfiguration"
            "-framework CoreFoundation"
    )
elseif (NETKIT_DKP)
endif()

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION include
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
        PATTERN "*.hxx"
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/netkitConfigVersion.cmake"
        VERSION ${CMAKE_PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
)

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/netkitConfig.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/netkitConfig.cmake"
        @ONLY
)

install(TARGETS netkit
        EXPORT netkitTargets
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)

if (NETKIT_DKP AND NETKIT_ENABLE_WOLFSSL)
    install(
        FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/include/netkit/stream/wolfssl/user_settings.h
        DESTINATION
        include/wolfssl/wolfcrypt/
    )
endif()

install(EXPORT netkitTargets
        FILE netkitTargets.cmake
        NAMESPACE netkit::
        DESTINATION lib/cmake/netkit
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/netkitConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/netkitConfigVersion.cmake"
        DESTINATION lib/cmake/netkit
)

if (NETKIT_ENABLE_TESTS)
    include(FetchContent)

    FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v3.12.0
    )

    FetchContent_MakeAvailable(Catch2)

    add_executable(netkit-tests
            tests/test.cpp
    )

    target_link_libraries(netkit-tests
            PRIVATE
            netkit
            Catch2::Catch2WithMain
    )

    include(Catch)
    catch_discover_tests(netkit-tests)
endif()

if (NETKIT_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
