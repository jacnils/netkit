#pragma once

#include <netkit/definitions.hpp>

namespace netkit::stream {

enum class NETKIT_API version {
	TLS_1_1,
	TLS_1_2,
	TLS_1_3
};

enum class NETKIT_API verification {
	peer,
	none
};

}