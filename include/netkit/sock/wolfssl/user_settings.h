#pragma once

#include <stdlib.h>
#include <time.h>
#include <ogc/lwp_watchdog.h>

// required to know wtf is wrong
#define DEBUG_WOLFSSL

#define NO_WRITEV
#define SINGLE_THREADED
#define DEVKITPRO
#define SIZEOF_LONG_LONG 8
#define BIG_ENDIAN_ORDER
#define NO_FILESYSTEM
#define WC_NO_HARDEN

// required for some sites
#define WOLFSSL_SNI
#define HAVE_SNI
#define HAVE_TLS_EXTENSIONS

// randomness... oh wait, random mess haha
#define NO_DEV_URANDOM
#define NO_DEV_RANDOM
#define CUSTOM_RAND_GENERATE gen_rand
#define CUSTOM_RAND_TYPE unsigned int

static unsigned int gen_rand(void) {
	uint64_t tick = gettime();
	srand(time(nullptr) ^ (unsigned int)tick);
	return rand() % 10000 + 1;
}

#define WOLFSSL_TLS12
#define WOLFSSL_TLS13
#define HAVE_TLS_EXTENSIONS
#define HAVE_SUPPORTED_CURVES
#define HAVE_FFDHE_2048
#define HAVE_HKDF
#define WC_RSA_PSS
#define HAVE_AEAD
#define HAVE_AESGCM
#define HAVE_ECC

#ifndef GEKKO
#define GEKKO
#endif