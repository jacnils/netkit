#include "internal.hpp"

#include <netkit/c/common.h>

namespace {
	thread_local std::string g_last_error;
}

void netkit::c::set_last_error(const std::string& msg) noexcept {
	try {
		g_last_error = msg;
	} catch (...) {}
}

extern "C" const char* nk_last_error(void) {
	return g_last_error.c_str();
}

extern "C" void nk_free_string(char* str) {
	std::free(str); // should we new/delete instead?
}
