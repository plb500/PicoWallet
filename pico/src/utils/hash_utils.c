#include "hash_utils.h"
#include <string.h>
#include "hashing/ripemd160.h"
#include "cryptography/cifra/src/sha2.h"

ripemd160_context ripemd160_ctx;

void double_256(const uint8_t *input, int buffer_size, uint8_t *output) {
    do_sha256(input, buffer_size, output);
    do_sha256(output, 32, output);
}

void do_sha256(const uint8_t *input, int buffer_size, uint8_t *output) {
    cf_hash(&cf_sha256, input, buffer_size, output);
}

void do_sha512(const uint8_t *input, int buffer_size, uint8_t *output) {
    cf_hash(&cf_sha512, input, buffer_size, output);
}

void hash_160(const uint8_t *input, int buffer_size, uint8_t *output) {
    uint8_t shaOutput[32];
    do_sha256(input, buffer_size, shaOutput);
    do_ripemd160(shaOutput, 32, output);
}

void do_ripemd160(const uint8_t *input, int inputSize, uint8_t *output) {
    ripemd160_hash(input, inputSize, output, &ripemd160_ctx);
}
