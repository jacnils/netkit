#ifndef NETKIT_UTILITY_H
#define NETKIT_UTILITY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <netkit-c/export.h>
#include <stddef.h>

NETKIT_C_API int netkit_network_is_ipv4(const char* ip, size_t len);
NETKIT_C_API int netkit_network_is_ipv6(const char* ip, size_t len);
NETKIT_C_API int netkit_network_is_valid_port(int port);
NETKIT_C_API int netkit_network_usable_ipv6_address_exists();

#ifdef __cplusplus
}
#endif
#endif