#pragma once

#if defined(NETKIT_ENABLE_OPENSSL) && defined(NETKIT_ENABLE_WOLFSSL)
#error "Only one SSL backend can be enabled at compile time"
#endif

#if defined(NETKIT_OPENSSL)
#include <netkit/sock/openssl/ssl_sync_sock.hpp>
#elif defined(NETKIT_WOLFSSL)
#include <netkit/sock/wolfssl/ssl_sync_sock.hpp>
#endif