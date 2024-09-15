#ifndef RIPEMD160_H
#define RIPEMD160_H

#include <stdint.h>

#define BLOCK_SIZE                  (64)
#define RIPEMD160_DIGEST_SIZE       (20)


typedef struct {
    union {
        uint8_t             m_bytes[BLOCK_SIZE];
        uint32_t            m_words[BLOCK_SIZE / 4];
    } m_buffer;
    uint32_t                m_chainingVariables[5];
    uint64_t                m_length;
    uint8_t                 m_bufferPosition;
} ripemd160_context;

void ripemd160_init(ripemd160_context *context);
void ripemd160_hash(const uint8_t *input, uint32_t inputLength, uint8_t *output, ripemd160_context *context);

#endif      // RIPEMD160_H
