#pragma once

#include <netkit/c/common.h>
#include <netkit/except.hpp>

#include <cstdlib>
#include <cstring>
#include <exception>
#include <string>
#include <utility>

namespace netkit::c {
	void set_last_error(const std::string& msg) noexcept;

	template <typename F>
	nk_status_t wrap(F&& f) noexcept {
		try {
			f();
			return NK_OK;
		} catch (const netkit::socket_error& e) {
			set_last_error(e.what());
			return NK_ERR_SOCKET;
		} catch (const netkit::parsing_error& e) {
			set_last_error(e.what());
			return NK_ERR_PARSING;
		} catch (const netkit::ip_error& e) {
			set_last_error(e.what());
			return NK_ERR_IP;
		} catch (const netkit::dns_error& e) {
			set_last_error(e.what());
			return NK_ERR_DNS;
		} catch (const netkit::ssl_error& e) {
			set_last_error(e.what());
			return NK_ERR_SSL;
		} catch (const netkit::length_error& e) {
			set_last_error(e.what());
			return NK_ERR_LENGTH;
		} catch (const netkit::logic_error& e) {
			set_last_error(e.what());
			return NK_ERR_LOGIC;
		} catch (const netkit::io_error& e) {
			set_last_error(e.what());
			return NK_ERR_IO;
		} catch (const netkit::generic_error& e) {
			set_last_error(e.what());
			return NK_ERR_GENERIC;
		} catch (const std::exception& e) {
			set_last_error(e.what());
			return NK_ERR_UNKNOWN;
		} catch (...) {
			set_last_error("unknown error");
			return NK_ERR_UNKNOWN;
		}
	}

	template <typename F>
	nk_status_t wrap_checked(bool args_ok, F&& f) noexcept {
		if (!args_ok) {
			set_last_error("invalid argument");
			return NK_ERR_INVALID_ARGUMENT;
		}
		return wrap(std::forward<F>(f));
	}

	inline char* dup_bytes(const char* data, std::size_t len) noexcept {
		auto* buf = static_cast<char*>(std::malloc(len + 1));
		if (!buf) return nullptr;
		if (len > 0) std::memcpy(buf, data, len);
		buf[len] = '\0';
		return buf;
	}

	inline char* dup_string(const std::string& s) noexcept {
		return dup_bytes(s.data(), s.size());
	}

}
