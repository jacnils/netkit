cmake_minimum_required(VERSION 3.30)
project(netkit LANGUAGES C CXX)

option(NETKIT_ENABLE_OPENSSL "Enable OpenSSL support in netkit" ON)
option(NETKIT_ENABLE_TESTS "Enable building tests" OFF)
option(NETKIT_ENABLE_C_BINDINGS "Enable experimental C bindings for netkit" ON)
option(NETKIT_ENABLE_C_TESTS "Enable C building tests" ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CTest)
if (NETKIT_ENABLE_TESTS)
    enable_testing()
endif()

add_library(netkit SHARED
        src/windows/windows_boilerplate.cpp
        src/utility.cpp
        src/network/utility.cpp
        src/network/local_address.cpp
        src/network/network_interface.cpp
        src/network/ip_list.cpp
        src/dns/record_type.cpp
        src/dns/nameserver_list.cpp
        src/sock/sock_addr.cpp
        src/dns/response_parser.cpp
        src/dns/query_builder.cpp
        src/sock/sock_peer.cpp
        src/sock/sync_sock.cpp
        src/dns/cache.cpp
        src/sock/openssl/ssl_sync_sock_openssl.cpp
        src/http/predefined.cpp
        src/http/sync_client.cpp
        src/crypto/windows/certs.cpp
)

set_target_properties(netkit PROPERTIES
        EXPORT_NAME netkit
        POSITION_INDEPENDENT_CODE ON
        VERSION 0.1.0
        SOVERSION 1
)

target_compile_definitions(netkit
        PRIVATE NETKIT_BUILD_DLL
)

target_include_directories(netkit
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

if (NETKIT_ENABLE_OPENSSL)
    find_package(OpenSSL)

    if (NOT OpenSSL_FOUND)
        message(STATUS "OpenSSL not found. You can install it via system package manager or vcpkg.")

        if (DEFINED ENV{VCPKG_ROOT})
            set(CMAKE_TOOLCHAIN_FILE
                    "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
                    CACHE STRING ""
            )
            find_package(OpenSSL REQUIRED)
        else()
            message(STATUS "Not using VCPKG")
        endif()
    endif()

    if (OpenSSL_FOUND)
        message(STATUS "OpenSSL enabled in netkit")
        target_compile_definitions(netkit PUBLIC NETKIT_OPENSSL)
        target_link_libraries(netkit PUBLIC OpenSSL::SSL OpenSSL::Crypto)
    else()
        message(FATAL_ERROR "OpenSSL requested but not found.")
    endif()
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
        VERSION 1.0.0
        COMPATIBILITY SameMajorVersion
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

if (NETKIT_ENABLE_C_BINDINGS)
    message(STATUS "C bindings enabled")
    add_library(netkit-c SHARED
            src/c/sock/sock_addr.cpp
            src/c/sock/sync_sock.cpp
    )

    set_target_properties(netkit-c PROPERTIES
            EXPORT_NAME netkit-c
            POSITION_INDEPENDENT_CODE ON
            CXX_VISIBILITY_PRESET hidden
            VISIBILITY_INLINES_HIDDEN ON
            VERSION 0.1.0
            SOVERSION 1
    )

    target_compile_definitions(netkit-c
            PRIVATE NETKIT_C_BUILD_DLL
    )

    target_link_libraries(netkit-c
            PUBLIC netkit
    )

    target_include_directories(netkit-c
            PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )

    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/netkit-c/
            DESTINATION include/netkit-c
            FILES_MATCHING
            PATTERN "*.h"
    )

    install(TARGETS netkit-c
            EXPORT netkitTargets
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
    )

    if (NETKIT_ENABLE_C_TESTS)
        message(STATUS "C API tests enabled")

        add_executable(netkit-c-tests
            tests/c/test_sock_addr.c
        )

        add_dependencies(netkit-c-tests
            netkit-c
        )

        target_link_libraries(netkit-c-tests
                PRIVATE netkit-c
        )

        add_test(NAME netkit-c-tests COMMAND netkit-c-tests)
    else()
        message(STATUS "C API tests disabled")
    endif()
else()
    message(STATUS "C bindings disabled")
endif()