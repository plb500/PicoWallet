#ifndef HASH_UTILS_H
#define HASH_UTILS_H

#include "pico/types.h"

#define SHA256_DIGEST_SIZE          (32)
#define SHA512_DIGEST_SIZE          (64)
#define RIPEMD_160_DIGEST_SIZE      (20)
#define PBKDF2_HMAC_SHA256_SIZE     (32)

void do_sha256(const uint8_t *input, int buffer_size, uint8_t *output);
void do_sha512(const uint8_t *input, int buffer_size, uint8_t *output);

void double_256(const uint8_t *input, int buffer_size, uint8_t *output);

void do_ripemd160(const uint8_t *input, int inputSize, uint8_t *output);

void hash_160(const uint8_t *input, int buffer_size, uint8_t *output);


#endif
