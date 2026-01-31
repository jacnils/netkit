#ifndef NETKIT_SOCK_ADDR_H
#define NETKIT_SOCK_ADDR_H

#if defined(__APPLE__)
#define NETKIT_MACOS 1
#endif
#if defined(__unix__) || defined(__unix) || defined(__APPLE__) && defined(__MACH__)
#define NETKIT_UNIX 1
#endif
#ifdef _WIN32
#define NETKIT_WINDOWS 1
#endif

#ifndef NETKIT_FALLBACK_IPV4_DNS_1
#define NETKIT_FALLBACK_IPV4_DNS_1 "8.8.8.8"
#endif
#ifndef NETKIT_FALLBACK_IPV4_DNS_2
#define NETKIT_FALLBACK_IPV4_DNS_2 "8.8.4.4"
#endif
#ifndef NETKIT_FALLBACK_IPV6_DNS_1
#define NETKIT_FALLBACK_IPV6_DNS_1 "2001:4860:4860::8888"
#endif
#ifndef NETKIT_FALLBACK_IPV6_DNS_2
#define NETKIT_FALLBACK_IPV6_DNS_2 "2001:4860:4860::8844"
#endif
#ifndef NETKIT_LOCALHOST_IPV4
#define NETKIT_LOCALHOST_IPV4 "127.0.0.1"
#endif
#ifndef NETKIT_LOCALHOST_IPV6
#define NETKIT_LOCALHOST_IPV6 "::1"
#endif

#endif