#pragma once

namespace netkit::stream {

enum class version {
	TLS_1_1,
	TLS_1_2,
	TLS_1_3
};

enum class verification {
	peer,
	none
};

}