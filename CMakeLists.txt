cmake_minimum_required(VERSION 3.28)
project(netkit LANGUAGES CXX VERSION 0.6.2)

option(NETKIT_ENABLE_WOLFSSL "Enable WolfSSL support in netkit" ON)
option(NETKIT_ENABLE_TESTS "Enable building tests" OFF)
option(NETKIT_ENABLE_WINDOWS_CERTSTORE "Enable getting CA certificates from the Windows store" ON)
option(NETKIT_ENABLE_FALLBACK_CA "Enable fallback CA certficate (required for DevkitPro)" ON)
option(NETKIT_ENABLE_EPOLL "Enable using epoll backend (Linux only)" ON)
option(NETKIT_ENABLE_WSAPOLL "Enable using wsapoll backend (Windows only)" ON)
option(NETKIT_ENABLE_KQUEUE "Enable using kqueue backend (macOS, BSD only)" ON)
option(NETKIT_ENABLE_HTTP "Enable netkit's HTTP abstractions" ON)
option(NETKIT_ENABLE_DNS "Enable netkit's DNS features" ON)
option(NETKIT_WOLFSSL_DEBUG "Enable WolfSSL debugging" OFF)
option(NETKIT_BUILD_SHARED "Build netkit as a shared library" ON)
option(NETKIT_BUILD_SHARED_WOLFSSL "Build WolfSSL as a shared library" OFF)
option(NETKIT_USE_SYSTEM_WOLFSSL "Use system WolfSSL if found (not recommended)" OFF)
option(NETKIT_BUILD_EXAMPLES "Build netkit's examples" OFF)
option(NETKIT_BUILD_MODULES "Build netkit's C++23 module interface (experimental)" OFF)
option(NETKIT_BUILD_C_BINDINGS "Build netkit C bindings" ON)
option(NETKIT_DEBUG "Build netkit with debugging" OFF)

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
    set(NETKIT_LIB_TYPE STATIC
            include/netkit/http/client.hpp)
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

set(NETKIT_SOURCE_FILES
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
        src/crypto/windows/certs.cpp
        src/body/stream_body.cpp
        src/http/multipart.cpp
        src/http/multipart_reader.cpp
        src/body/multipart_part_body.cpp
        src/io/io_awaitable.cpp
        src/io/linux/io_backend.cpp
        src/io/bsd/io_backend.cpp
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
        src/io/timeout.cpp
        src/stream/utility.cpp
        src/stream/wolfssl/async_tls_stream.cpp
)

set(NETKIT_C_BINDING_SOURCE_FILES
        src/c/common.cpp
        src/c/socket.cpp
        src/c/io.cpp
        src/c/stream.cpp
        src/c/body.cpp
        src/c/tcp.cpp
        src/c/udp.cpp
        src/c/uds.cpp
        src/c/tls.cpp
        src/c/interface.cpp
        src/c/http.cpp
        src/c/http_server.cpp
)

if (NETKIT_BUILD_C_BINDINGS)
    list(APPEND NETKIT_SOURCE_FILES ${NETKIT_C_BINDING_SOURCE_FILES})
endif()

set(NETKIT_SOURCE_MODULES
        modules/netkit.cppm
        modules/netkit-except.cppm
        modules/netkit-utility.cppm
        modules/netkit-network.cppm
        modules/netkit-socket.cppm
        modules/netkit-io.cppm
        modules/netkit-stream.cppm
        modules/netkit-body.cppm
        modules/netkit-tcp.cppm
        modules/netkit-datagram.cppm
        modules/netkit-udp.cppm
        modules/netkit-uds.cppm
        modules/netkit-dns.cppm
        modules/netkit-http.cppm
        modules/netkit-crypto.cppm
)

add_library(netkit ${NETKIT_LIB_TYPE}
    ${NETKIT_SOURCE_FILES}
)

add_library(netkit::netkit ALIAS netkit)

target_compile_options(netkit PRIVATE
        $<$<CONFIG:Release>:-O3>
)

set_target_properties(netkit PROPERTIES
        EXPORT_NAME netkit
        POSITION_INDEPENDENT_CODE ON
        VERSION ${CMAKE_PROJECT_VERSION}
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

    if (NOT NETKIT_BUILD_SHARED OR NOT NETKIT_BUILD_SHARED_WOLFSSL)
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

        set_target_properties(wolfssl PROPERTIES
                POSITION_INDEPENDENT_CODE ON
        )

        set(WOLFSSL_TARGET wolfssl)
    else()
        set(WOLFSSL_TARGET wolfssl)
    endif()

    target_compile_definitions(netkit PUBLIC NETKIT_WOLFSSL)
    target_link_libraries(netkit PUBLIC ${WOLFSSL_TARGET})

    message(STATUS "WolfSSL enabled in netkit")
endif()

if (NETKIT_BUILD_MODULES)
    if (CMAKE_VERSION VERSION_LESS "3.30")
        message(FATAL_ERROR "NETKIT_BUILD_MODULES requires CMake >= 3.30 (found ${CMAKE_VERSION})")
    endif()

    if (NOT CMAKE_GENERATOR MATCHES "Ninja")
        message(WARNING "NETKIT_BUILD_MODULES: CMake's C++ module dependency scanning currently only "
                "works reliably with the Ninja generators. Your generator is '${CMAKE_GENERATOR}'.")
    endif()

    target_sources(netkit
            PUBLIC
            FILE_SET netkit_modules
            TYPE CXX_MODULES
            BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/modules
            FILES
            ${NETKIT_SOURCE_MODULES}
    )
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

if (NETKIT_DEBUG)
    target_compile_definitions(netkit PUBLIC NETKIT_DEBUG)
endif()

if (NETKIT_ENABLE_WOLFSSL OR NETKIT_ENABLE_OPENSSL)
    target_compile_definitions(netkit PUBLIC NETKIT_SSL)
endif()

if (NETKIT_BUILD_SHARED)
    target_compile_definitions(netkit PUBLIC NETKIT_SHARED)
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

if (NETKIT_ENABLE_KQUEUE)
    target_compile_definitions(netkit PUBLIC NETKIT_KQUEUE)
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

if (NETKIT_BUILD_MODULES)
install(TARGETS netkit
        EXPORT netkitTargets
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        FILE_SET netkit_modules DESTINATION include
)
else()
    install(TARGETS netkit
            EXPORT netkitTargets
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
    )
endif()

if (NETKIT_DKP AND NETKIT_ENABLE_WOLFSSL)
    install(
        FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/include/netkit/stream/wolfssl/user_settings.h
        DESTINATION
        include/wolfssl/wolfcrypt/
    )
endif()

function(netkit_copy_dll target)
    if (NOT WIN32 OR NOT NETKIT_BUILD_SHARED)
        return()
    endif()

    add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:netkit>"
            "$<TARGET_FILE_DIR:${target}>"

            VERBATIM
    )

    if (TARGET wolfssl::wolfssl AND NOT NETKIT_BUILD_SHARED_WOLFSSL)
        add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:wolfssl::wolfssl>"
                "$<TARGET_FILE_DIR:${target}>"
                VERBATIM
        )
    endif()
endfunction()

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

    if (WIN32 AND NETKIT_BUILD_SHARED)
        add_custom_command(TARGET netkit-tests POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_RUNTIME_DLLS:netkit-tests>
                "$<TARGET_FILE_DIR:netkit-tests>"
                COMMAND_EXPAND_LISTS
                VERBATIM
        )
    endif()

    include(Catch)
    catch_discover_tests(netkit-tests)
endif()

if (NETKIT_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
