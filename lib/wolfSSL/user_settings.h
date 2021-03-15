/* Generated wolfSSL user_settings.h file for Arduino */
#ifndef ARDUINO_USER_SETTINGS_H
#define ARDUINO_USER_SETTINGS_H

#if defined(ARDUINO) && defined(ESP32)
#define WOLFSSL_ARDUINO
#define WOLFSSL_USER_SETTINGS

#define WOLFCRYPT_HAVE_SRP 
#define WOLFSSL_SHA512 
#define WOLFSSL_BASE64_ENCODE 
#define NO_MD5 
#define NO_SHA 
#define HAVE_HKDF 
#define HAVE_CHACHA 
#define HAVE_POLY1305 
#define HAVE_ED25519 
#define HAVE_CURVE25519 
#define NO_SESSION_CACHE 
//#define USE_WOLFSSL_MEMORY 
#define RSA_LOW_MEM 
#define GCM_SMALL 
#define USE_SLOW_SHA512 
#define WOLFCRYPT_ONLY 
//#define WOLFSSL_ESPIDF

#include <esp_system.h>

static inline int hwrand_generate_block(uint8_t *buf, size_t len) {
	int i;
	for (i = 0; i + 4 < len; i += 4) {
		*((uint32_t*)buf) = esp_random();
		buf += 4;
	}
	if (i < len) {
		uint32_t r = esp_random();
		while (i < len) {
			*buf++ = r;
			r >>= 8;
			i++;
		}
	}

	return 0;
}
#define CUSTOM_RAND_GENERATE_BLOCK hwrand_generate_block


#define NO_ASN_TIME
#define XTIME time_t 
#define XGMTIME 

#define WC_SHA_DIGEST_SIZE 20
#endif



#if defined(ARDUINO) && defined(ESP8266)

#define WOLFSSL_USER_SETTINGS

//#include <esp/hwrand.h>

#include "stdint.h"
#include "stddef.h"
#include "stdlib.h"
#include "osapi.h"

static inline int hwrand_generate_block(uint8_t *buf, size_t len) {
	os_get_random(buf, len);
	return 0;
}

//#define FREERTOS
#define WC_NO_HARDEN
#define NO_WOLFSSL_DIR
#define SINGLE_THREADED
#define WOLFSSL_LWIP
#define NO_INLINE

#define NO_WOLFSSL_MEMORY
//#define NO_WOLFSSL_SMALL_STACK
//#define WOLFSSL_SMALL_STACK
#define MP_LOW_MEM

#define CUSTOM_RAND_GENERATE_BLOCK hwrand_generate_block

//==========
// added
//==========
#define WOLFCRYPT_ONLY

#define NO_WRITEV
#define NO_ASN
#define NO_AES
#define NO_RC4
#define NO_RSA
#define NO_SHA256
#define NO_DH
#define NO_DSA
#define NO_CODING
#define NO_SHA
#define NO_MD5

#define HAVE_CURVE25519
#define HAVE_CHACHA
#define HAVE_POLY1305
#define HAVE_ED25519
#define WOLFSSL_SHA512
#define WOLFCRYPT_HAVE_SRP
#define HAVE_HKDF
//#define CURVE25519_SMALL
//#define ED25519_SMALL  //关联ED25519，关闭这个之后编译dram会超（stack mem太大）
//`.bss' is not within region `dram0_0_seg'

#define WC_NO_HASHDRBG

#define WOLFSSL_BASE64_ENCODE

//see integer.c
//default winsize=5(MP_LOW_MEM), but ram(heap) is not sufficient!
//winsize of {2,3,4,5} are same performance
//lower winsize, lower ram required
#define ESP_INTEGER_WINSIZE 2

//force use s_mp_exptmod (lower memory), and smiller performance with mp_exptmod_fast
#define ESP8266_FORCE_S_MP_EXPTMOD

//winsize=3 & mp_exptmod_fast : ram(heap) is not sufficient

#define MP_16BIT //faster than 32bit in ESP8266

// crypto.c  crypto_ed25519_verify
// No sufficient ram to run ge_double_scalarmult_vartime in ge_operations.c
// Run the ge_double_scalarmult_vartime in ge_low_mem
#define ESP8266_LOWMEM_FORCE_IMPL



#endif
#endif /* ARDUINO_USER_SETTINGS_H */
