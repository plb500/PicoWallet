#ifndef HASH_UTILS_H
#define HASH_UTILS_H

#include <stdint.h>
void double_256(const uint8_t *input, int buffer_size, uint8_t *output);

void do_sha256(const uint8_t *input, int buffer_size, uint8_t *output);
void do_sha512(const uint8_t *input, int buffer_size, uint8_t *output);

void hash_160(const uint8_t *input, int buffer_size, uint8_t *output);

void do_ripemd160(const uint8_t *input, int inputSize, uint8_t *output);

#endif
