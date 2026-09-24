#include "internal.hpp"

#include <netkit/c/interface.h>

#include <netkit/network/network_interface.hpp>

using netkit::network::network_interface;

namespace {

	nk_local_ipv4_t to_c(const netkit::network::local_ip_address_v4& a) {
		return nk_local_ipv4_t{
			netkit::c::dup_string(a.get_ip()), netkit::c::dup_string(a.get_netmask()),
			netkit::c::dup_string(a.get_broadcast()), netkit::c::dup_string(a.get_peer()),
			a.is_loopback(), a.is_multicast()
		};
	}

	nk_local_ipv6_t to_c(const netkit::network::local_ip_address_v6& a) {
		return nk_local_ipv6_t{
			netkit::c::dup_string(a.get_ip()), netkit::c::dup_string(a.get_netmask()),
			a.is_loopback(), a.is_multicast(), a.is_link_local(), netkit::c::dup_string(a.get_scope_id())
		};
	}

} // namespace

extern "C" nk_status_t nk_network_get_interfaces(nk_network_interface_t** out, size_t* out_count) {
	return netkit::c::wrap_checked(out != nullptr && out_count != nullptr, [&] {
		auto ifaces = netkit::network::get_interfaces();

		auto* arr = static_cast<nk_network_interface_t*>(std::calloc(ifaces.empty() ? 1 : ifaces.size(),
		                                                              sizeof(nk_network_interface_t)));
		if (!arr) throw netkit::generic_error("out of memory");

		for (std::size_t i = 0; i < ifaces.size(); ++i) {
			const auto& iface = ifaces[i];
			nk_network_interface_t entry{};

			entry.name = netkit::c::dup_string(iface.get_name());
			entry.up = iface.is_up();
			entry.running = iface.is_running();
			entry.broadcast = iface.is_broadcast();
			entry.point_to_point = iface.is_point_to_point();

			const auto& v4 = iface.get_ipv4_addrs();
			entry.ipv4_count = v4.size();
			if (!v4.empty()) {
				entry.ipv4 = static_cast<nk_local_ipv4_t*>(std::malloc(sizeof(nk_local_ipv4_t) * v4.size()));
				if (!entry.ipv4) throw netkit::generic_error("out of memory");
				for (std::size_t j = 0; j < v4.size(); ++j) entry.ipv4[j] = to_c(v4[j]);
			}

			const auto& v6 = iface.get_ipv6_addrs();
			entry.ipv6_count = v6.size();
			if (!v6.empty()) {
				entry.ipv6 = static_cast<nk_local_ipv6_t*>(std::malloc(sizeof(nk_local_ipv6_t) * v6.size()));
				if (!entry.ipv6) throw netkit::generic_error("out of memory");
				for (std::size_t j = 0; j < v6.size(); ++j) entry.ipv6[j] = to_c(v6[j]);
			}

			arr[i] = entry;
		}

		*out = arr;
		*out_count = ifaces.size();
	});
}

extern "C" void nk_network_interfaces_free(nk_network_interface_t* interfaces, size_t count) {
	if (!interfaces) return;
	for (std::size_t i = 0; i < count; ++i) {
		auto& entry = interfaces[i];
		std::free(entry.name);
		for (std::size_t j = 0; j < entry.ipv4_count; ++j) {
			std::free(entry.ipv4[j].ip);
			std::free(entry.ipv4[j].netmask);
			std::free(entry.ipv4[j].broadcast);
			std::free(entry.ipv4[j].peer);
		}
		std::free(entry.ipv4);
		for (std::size_t j = 0; j < entry.ipv6_count; ++j) {
			std::free(entry.ipv6[j].ip);
			std::free(entry.ipv6[j].netmask);
			std::free(entry.ipv6[j].scope_id);
		}
		std::free(entry.ipv6);
	}
	std::free(interfaces);
}
