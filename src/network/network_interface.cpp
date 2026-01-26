/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file network_interface.cpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @brief Implementation of the network interface class.
 */
#include <vector>
#include <unordered_map>
#include <ranges>
#include <stdexcept>

#include <netkit/network/network_interface.hpp>
#include <netkit/definitions.hpp>
#include <netkit/except.hpp>

#ifdef NETKIT_WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <unordered_set>
#endif
#ifdef NETKIT_UNIX
#include <ifaddrs.h>
#include <net/if.h>
#include <arpa/inet.h>
#endif

const std::vector<netkit::network::local_ip_address_v4>& netkit::network::network_interface::get_ipv4_addrs() const {
    return this->ipv4;
}

const std::vector<netkit::network::local_ip_address_v6>& netkit::network::network_interface::get_ipv6_addrs() const {
    return this->ipv6;
}

std::string netkit::network::network_interface::get_name() const {
    return this->name;
}

bool netkit::network::network_interface::is_up() const {
    return this->up;
}

bool netkit::network::network_interface::is_running() const {
    return this->running;
}

bool netkit::network::network_interface::is_broadcast() const {
    return this->broadcast;
}

bool netkit::network::network_interface::is_point_to_point() const {
    return this->point_to_point;
}

void netkit::network::network_interface::set_running(bool value) {
    this->running = value;
}

void netkit::network::network_interface::set_broadcast(bool value) {
    this->broadcast = value;
}

void netkit::network::network_interface::set_point_to_point(bool value) {
    this->point_to_point = value;
}

void netkit::network::network_interface::set_name(const std::string& value) {
    this->name = value;
}

void netkit::network::network_interface::set_up(bool value) {
    this->up = value;
}

void netkit::network::network_interface::set_ipv4_addrs(const std::vector<local_ip_address_v4>& ipv4_addrs) {
    this->ipv4 = ipv4_addrs;
}

void netkit::network::network_interface::set_ipv6_addrs(const std::vector<local_ip_address_v6>& ipv6_addrs) {
    this->ipv6 = ipv6_addrs;
}

#ifdef NETKIT_UNIX
std::vector<netkit::network::network_interface> netkit::network::get_interfaces() {
    std::vector<netkit::network::network_interface> list;

    struct ifaddrs* ifaddr;
    if (getifaddrs(&ifaddr) == -1) {
        throw ip_error{"getifaddrs() failed in get_interfaces()"};
    }

    std::unordered_map<std::string, netkit::network::network_interface> iface_map;

    for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr)
            continue;

        const std::string name(ifa->ifa_name);
        auto& iface = iface_map[name];
        iface.name = name;

        iface.up = (ifa->ifa_flags & IFF_UP);
        iface.running = (ifa->ifa_flags & IFF_RUNNING);
        iface.broadcast = (ifa->ifa_flags & IFF_BROADCAST);
        iface.point_to_point = (ifa->ifa_flags & IFF_POINTOPOINT);

        char addr_buf[INET6_ADDRSTRLEN]{};
        char netmask_buf[INET6_ADDRSTRLEN]{};
        char broadcast_buf[INET_ADDRSTRLEN]{};
        char peer_buf[INET_ADDRSTRLEN]{};

        if (ifa->ifa_addr->sa_family == AF_INET) {
            auto sa = reinterpret_cast<struct sockaddr_in*>(ifa->ifa_addr);
            if (!inet_ntop(AF_INET, &(sa->sin_addr), addr_buf, sizeof(addr_buf)))
                continue;

            if (ifa->ifa_netmask) {
                auto netmask = reinterpret_cast<struct sockaddr_in*>(ifa->ifa_netmask);
                inet_ntop(AF_INET, &(netmask->sin_addr), netmask_buf, sizeof(netmask_buf));
            }

            if (ifa->ifa_flags & IFF_BROADCAST && ifa->ifa_broadaddr) {
                auto broad = reinterpret_cast<struct sockaddr_in*>(ifa->ifa_broadaddr);
                inet_ntop(AF_INET, &(broad->sin_addr), broadcast_buf, sizeof(broadcast_buf));
            }

            if (ifa->ifa_flags & IFF_POINTOPOINT && ifa->ifa_dstaddr) {
                auto peer = reinterpret_cast<struct sockaddr_in*>(ifa->ifa_dstaddr);
                inet_ntop(AF_INET, &(peer->sin_addr), peer_buf, sizeof(peer_buf));
            }

            local_ip_address_v4 addr{
                std::string(addr_buf),
                std::string(netmask_buf),
                std::string(broadcast_buf),
                std::string(peer_buf),
                (ifa->ifa_flags & IFF_LOOPBACK) != 0,
                (ifa->ifa_flags & IFF_MULTICAST) != 0
            };

            iface.ipv4.emplace_back(std::move(addr));
        } else if (ifa->ifa_addr->sa_family == AF_INET6) {
            auto sa6 = reinterpret_cast<struct sockaddr_in6*>(ifa->ifa_addr);
            if (!inet_ntop(AF_INET6, &(sa6->sin6_addr), addr_buf, sizeof(addr_buf)))
                continue;

            if (ifa->ifa_netmask) {
                auto netmask6 = reinterpret_cast<struct sockaddr_in6*>(ifa->ifa_netmask);
                inet_ntop(AF_INET6, &(netmask6->sin6_addr), netmask_buf, sizeof(netmask_buf));
            }

            std::string scope_id_str;
            if (sa6->sin6_scope_id != 0) {
                scope_id_str = std::to_string(sa6->sin6_scope_id);
            }

            local_ip_address_v6 addr6{
                std::string(addr_buf),
                std::string(netmask_buf),
                (ifa->ifa_flags & IFF_LOOPBACK) != 0,
                (ifa->ifa_flags & IFF_MULTICAST) != 0,
                IN6_IS_ADDR_LINKLOCAL(&sa6->sin6_addr),
                std::move(scope_id_str)
            };

            iface.ipv6.emplace_back(std::move(addr6));
        }
    }

    freeifaddrs(ifaddr);

    list.reserve(iface_map.size());
    for (auto& snd: iface_map | std::views::values) {
        list.emplace_back(std::move(snd));
    }

    return list;
}
#endif
#ifdef NETKIT_WINDOWS
std::vector<netkit::network::network_interface> netkit::network::get_interfaces() {
    std::vector<netkit::network::network_interface> list;
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
    ULONG family = AF_UNSPEC;

    ULONG out_buf_len = 15000;
    std::vector<char> buffer(out_buf_len);

    auto* adapters = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(buffer.data());

    if (GetAdaptersAddresses(family, flags, nullptr, adapters, &out_buf_len) == ERROR_BUFFER_OVERFLOW) {
        buffer.resize(out_buf_len);
        adapters = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(buffer.data());
    }

    DWORD ret = GetAdaptersAddresses(family, flags, nullptr, adapters, &out_buf_len);
    if (ret != NO_ERROR) {
        throw std::runtime_error("GetAdaptersAddresses() failed in get_interfaces()");
    }

    for (IP_ADAPTER_ADDRESSES* adapter = adapters; adapter != nullptr; adapter = adapter->Next) {
        netkit::network::network_interface iface;

        iface.name = adapter->AdapterName;
        iface.up = (adapter->OperStatus == IfOperStatusUp);
        iface.running = iface.up;
        iface.broadcast = true;
        iface.point_to_point = false;

        for (IP_ADAPTER_UNICAST_ADDRESS* unicast = adapter->FirstUnicastAddress; unicast != nullptr; unicast = unicast->Next) {
            SOCKADDR* addr = unicast->Address.lpSockaddr;
            char addr_str[INET6_ADDRSTRLEN] = {};
            char netmask_str[INET6_ADDRSTRLEN] = {};

            if (addr->sa_family == AF_INET) {
                auto sa = reinterpret_cast<sockaddr_in*>(addr);
                inet_ntop(AF_INET, &sa->sin_addr, addr_str, sizeof(addr_str));

                ULONG mask = (unicast->OnLinkPrefixLength == 0) ? 0 : 0xFFFFFFFF << (32 - unicast->OnLinkPrefixLength);
                in_addr netmask{};
                netmask.S_un.S_addr = htonl(mask);
                inet_ntop(AF_INET, &netmask, netmask_str, sizeof(netmask_str));

                iface.ipv4.emplace_back(
                    std::string(addr_str),
                    std::string(netmask_str),
                    "",
                    "",
                    (adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK),
                    true
                );

            } else if (addr->sa_family == AF_INET6) {
                auto sa6 = reinterpret_cast<sockaddr_in6*>(addr);
                inet_ntop(AF_INET6, &sa6->sin6_addr, addr_str, sizeof(addr_str));

                iface.ipv6.emplace_back(
                    std::string(addr_str),
                    "",
                    (adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK),
                    true,
                    static_cast<bool>(IN6_IS_ADDR_LINKLOCAL(&sa6->sin6_addr)),
                    std::to_string(sa6->sin6_scope_id)
                );
            }
        }

        list.emplace_back(std::move(iface));
    }

    return list;
}
#endif
