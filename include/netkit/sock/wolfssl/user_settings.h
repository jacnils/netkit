#pragma once

#include <stdlib.h>
#include <time.h>
#include <ogc/lwp_watchdog.h>

#define NO_WRITEV
#define SINGLE_THREADED
#define DEVKITPRO
#define SIZEOF_LONG_LONG 8
#define BIG_ENDIAN_ORDER
#define NO_FILESYSTEM

#define NO_DEV_URANDOM
#define NO_DEV_RANDOM
#define CUSTOM_RAND_GENERATE_SEED gen_seed

static int gen_seed(unsigned char* output, unsigned int sz) {
	static uint32_t state = 0;
	if (state == 0) {
		state = (uint32_t)gettime();
		state ^= (uintptr_t)&state;
		state ^= (uintptr_t)&output;
		state ^= state << 13;
		state ^= state >> 17;
		state ^= state << 5;
	}

	for (unsigned int i = 0; i < sz; i++) {
		state ^= state << 13;
		state ^= state >> 17;
		state ^= state << 5;

		if ((i & 7) == 0)
			state ^= (uint32_t)gettime();

		output[i] = state & 0xFF;
	}

	return 0;
}

#define HAVE_HASHDRBG
#define OPENSSL_COEXIST

/* uncomment if wolfssl debugging is desired
#define DEBUG_WOLFSSL
*/
#define WOLFSSL_ALT_CERT_CHAINS

#define WOLFSSL_SNI
#define HAVE_SNI
#define HAVE_TLS_EXTENSIONS

#define WOLFSSL_TLS10
#define WOLFSSL_TLS11
#define WOLFSSL_TLS12
#define WOLFSSL_TLS13

#define HAVE_SUPPORTED_CURVES
#define HAVE_ECC
#define HAVE_ECC384
#define HAVE_HKDF
#define HAVE_AESGCM
#define HAVE_AEAD
#define WC_RSA_PSS
#define HAVE_RSA
#define HAVE_FFDHE_4096

#define TFM_TIMING_RESISTANT
#define ECC_TIMING_RESISTANT
#define WC_RSA_BLINDING

#ifndef GEKKO
#define GEKKO
#endif