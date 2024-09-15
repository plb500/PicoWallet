#include "ripemd160.h"
#include <string.h>

// Set this to 1 if we are on a big-endian system, 0 otherwise
#define BIG_ENDIAN_SYSTEM       (0)


// Binary/boolean functions
#define ROL(s, n)               (((n) << (s)) | ((n) >> (32-(s))))        // Cyclic left-shift the 32-bit word n left by s bits

#define F1(x, y, z)             ((x) ^ (y) ^ (z))
#define F2(x, y, z)             (((x) & (y)) | (~(x) & (z)))
#define F3(x, y, z)             (((x) | ~(y)) ^ (z))
#define F4(x, y, z)             (((x) & (z)) | ((y) & ~(z)))
#define F5(x, y, z)             ((x) ^ ((y) | ~(z)))


// Values to initialize the chaining variables with
static const uint32_t INITIAL_CHAINING_VALUES[5] = {
    0x67452301u,
    0xEFCDAB89u,
    0x98BADCFEu,
    0x10325476u,
    0xC3D2E1F0u
};

// Round constants, left line
static const uint32_t KL[5] = {
    0x00000000u,    // Round 1: 0
    0x5A827999u,    // Round 2: floor(2**30 * sqrt(2))
    0x6ED9EBA1u,    // Round 3: floor(2**30 * sqrt(3))
    0x8F1BBCDCu,    // Round 4: floor(2**30 * sqrt(5))
    0xA953FD4Eu     // Round 5: floor(2**30 * sqrt(7))
};

// Round constants, right line
static const uint32_t KR[5] = {
    0x50A28BE6u,    // Round 1: floor(2**30 * cubert(2))
    0x5C4DD124u,    // Round 2: floor(2**30 * cubert(3))
    0x6D703EF3u,    // Round 3: floor(2**30 * cubert(5))
    0x7A6D76E9u,    // Round 4: floor(2**30 * cubert(7))
    0x00000000u     // Round 5: 0
};


// Byte permutation lookup tables. Based on the following rho(i) and pi(i) permutations:
//
// rho(i) : { 7, 4, 13, 1, 10, 6, 15, 3, 12, 0, 9, 5, 2, 14, 11, 8 }[i]  0 <= i <= 15
//  pi(i) : 9*i + 5 (mod 16)
//
// Round permutation algorithm:
//
//    +-------+----------+----------+----------+----------+----------+
//    | LINE  |  Round 1 |  Round 2 |  Round 3 |  Round 4 |  Round 5 |
//    +-------+----------+----------+----------+----------+----------+
//    | Left  |    id    |    rho   |  rho^2   |  rho^3   |  rho^4   |
//    +-------+----------+----------+----------+----------+----------+
//    | Right |    pi    |  rho.pi  | rho^2.pi | rho^3.pi | rho^4.pi |
//    +-------+----------+----------+----------+----------+----------+

// Left
static const uint8_t RL[5][16] = {
    {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 },   // id
    {  7,  4, 13,  1, 10,  6, 15,  3, 12,  0,  9,  5,  2, 14, 11,  8 },   // rho
    {  3, 10, 14,  4,  9, 15,  8,  1,  2,  7,  0,  6, 13, 11,  5, 12 },   // rho^2
    {  1,  9, 11, 10,  0,  8, 12,  4, 13,  3,  7, 15, 14,  5,  6,  2 },   // rho^3
    {  4,  0,  5,  9,  7, 12,  2, 10, 14,  1,  3,  8, 11,  6, 15, 13 }    // rho^4
};

// Right
static const uint8_t RR[5][16] = {
    {  5, 14,  7,  0,  9,  2, 11,  4, 13,  6, 15,  8,  1, 10,  3, 12 },   // pi
    {  6, 11,  3,  7,  0, 13,  5, 10, 14, 15,  8, 12,  4,  9,  1,  2 },   // rho.pi
    { 15,  5,  1,  3,  7, 14,  6,  9, 11,  8, 12,  2, 10,  0,  4, 13 },   // rho^2.pi
    {  8,  6,  4,  1,  3, 11, 15,  0,  5, 12,  2, 13,  9,  7, 10, 14 },   // rho^3.pi
    { 12, 15, 10,  4,  1,  5,  8,  7,  6,  2, 13, 14,  0,  3,  9, 11 }    // rho^4.pi
};


// Shifts
//
//    +---------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
//    |  Round  | X0 | X1 | X2 | X3 | X4 | X5 | X6 | X7 | X8 | X9 | X10| X11| X12| X13| X14| X15|
//    +---------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
//    | Round 1 | 11 | 14 | 15 | 12 |  5 |  8 |  7 |  9 | 11 | 13 | 14 | 15 |  6 |  7 |  9 |  8 |
//    | Round 2 | 12 | 13 | 11 | 15 |  6 |  9 |  9 |  7 | 12 | 15 | 11 | 13 |  7 |  8 |  7 |  7 |
//    | Round 3 | 13 | 15 | 14 | 11 |  7 |  7 |  6 |  8 | 13 | 14 | 13 | 12 |  5 |  5 |  6 |  9 |
//    | Round 4 | 14 | 11 | 12 | 14 |  8 |  6 |  5 |  5 | 15 | 12 | 15 | 14 |  9 |  9 |  8 |  6 |
//    | Round 5 | 15 | 12 | 13 | 13 |  9 |  5 |  8 |  6 | 14 | 11 | 12 | 11 |  8 |  6 |  5 |  5 |
//    +---------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
//

// Left line shifts
static const uint8_t SL[5][16] = {
    { 11, 14, 15, 12,  5,  8,  7,  9, 11, 13, 14, 15,  6,  7,  9,  8 },     // Round 1
    {  7,  6,  8, 13, 11,  9,  7, 15,  7, 12, 15,  9, 11,  7, 13, 12 },     // Round 2
    { 11, 13,  6,  7, 14,  9, 13, 15, 14,  8, 13,  6,  5, 12,  7,  5 },     // Round 3
    { 11, 12, 14, 15, 14, 15,  9,  8,  9, 14,  5,  6,  8,  6,  5, 12 },     // Round 4
    {  9, 15,  5, 11,  6,  8, 13, 12,  5, 12, 13, 14, 11,  8,  5,  6 }      // Round 5
};

// Right line shifts
static const uint8_t SR[5][16] = {
    {  8,  9,  9, 11, 13, 15, 15,  5,  7,  7,  8, 11, 14, 14, 12,  6 },     // Round 1
    {  9, 13, 15,  7, 12,  8,  9, 11,  7,  7, 12,  7,  6, 15, 13, 11 },     // Round 2
    {  9,  7, 15, 11,  8,  6,  6, 14, 12, 13,  5, 14, 13, 13,  7,  5 },     // Round 3
    { 15,  5,  8, 11, 14, 14,  6, 14,  6,  9, 12,  9, 12,  5, 15,  8 },     // Round 4
    {  8,  5, 12,  9, 12,  5, 14,  6,  8, 13,  6,  5, 15, 13, 11, 11 }      // Round 5
};

static inline void byteswapInt(uint32_t *i) {
    union {
        uint32_t w;
        uint8_t b[4];
    } x, y;

    x.w = *i;
    y.b[0] = x.b[3];
    y.b[1] = x.b[2];
    y.b[2] = x.b[1];
    y.b[3] = x.b[0];
    *i = y.w;

    /* Wipe temporary variables */
    x.w = y.w = 0;
}

static inline void byteswap_digest(uint32_t *p) {
    int i;
    for(i = 0; i < 4; i++) {
        byteswapInt(p++);
        byteswapInt(p++);
        byteswapInt(p++);
        byteswapInt(p++);
    }
}


void process(const uint8_t *input, uint32_t length, ripemd160_context *context);
void finish(uint8_t *output, ripemd160_context *context);
void compress(ripemd160_context *context);

void process(const uint8_t *input, uint32_t length, ripemd160_context *context) {
    uint32_t bytesRemaining     = length;
    const uint8_t *inputPtr           = input;
    int i;

    context->m_bufferPosition   = 0;

    // Loop while we have enough full blocks to process
    while(bytesRemaining >= BLOCK_SIZE) {
        // Process the next block from the input stream
        for(i = 0; i < BLOCK_SIZE; ++i) {
            context->m_buffer.m_bytes[i] = inputPtr[i];
        }
//        memcpy(outputPtr, inputPtr, BLOCK_SIZE);
        compress(context);

        // Clear buffer values for next block
        memset(&(context->m_buffer), 0, sizeof(context->m_buffer));

        // Increment/decrement relevant counters
        inputPtr            += BLOCK_SIZE;
        context->m_length   += 512;
        bytesRemaining      -= BLOCK_SIZE;
    }
    if(bytesRemaining) {
        // Looks like there were some bytes left over.
        // We'll copy them into our buffer and deal with
        // them in the finish() step
        memcpy(&(context->m_buffer.m_bytes), inputPtr, bytesRemaining);
        context->m_bufferPosition   = bytesRemaining;
        context->m_length           += (bytesRemaining << 3);
    }
}


void ripemd160_init(ripemd160_context *context) {
    memcpy(context->m_chainingVariables, INITIAL_CHAINING_VALUES, RIPEMD160_DIGEST_SIZE);
    memset(&(context->m_buffer), 0, sizeof(context->m_buffer));
    context->m_length            = 0;
    context->m_bufferPosition    = 0;
}

void ripemd160_hash(const uint8_t *input, uint32_t inputLength, uint8_t *output, ripemd160_context *context) {
    ripemd160_init(context);
    process(input, inputLength, context);
    finish(output, context);
}

void finish(uint8_t *output, ripemd160_context *context) {
    // Append the padding
    context->m_buffer.m_bytes[context->m_bufferPosition++]      = 0x80;

    // If we don't have enough space for the length bytes then just
    // compress the current buffer and put the length bytes in a new one
    if(context->m_bufferPosition > 56) {
        compress(context);

        // Reset buffer
        memset(&(context->m_buffer), 0, sizeof(context->m_buffer));
        context->m_bufferPosition = 0;
    }

    // Add the length into the buffer and perform final compression
    context->m_buffer.m_words[14] = (uint32_t) (context->m_length & 0xFFFFffffu);
    context->m_buffer.m_words[15] = (uint32_t) ((context->m_length >> 32) & 0xFFFFffffu);

#if BIG_ENDIAN_SYSTEM
    byteswapInt(&(context->m_buffer.m_words[14]));
    byteswapInt(&(context->m_buffer.m_words[15]));
#endif

    compress(context);

#if BIG_ENDIAN_SYSTEM
    byteswap_digest(context->m_chainingVariables);
#endif

    // Copy digest into output buffer
    memcpy(output, context->m_chainingVariables, RIPEMD160_DIGEST_SIZE);
}

void compress(ripemd160_context *context) {
    uint8_t b           = 0;
    uint8_t round       = 0;
    uint32_t tempInt    = 0;
    uint32_t AL, BL, CL, DL, EL;            // Left line registers
    uint32_t AR, BR, CR, DR, ER;            // Right line registers

#if BIG_ENDIAN_SYSTEM
    byteswap_digest(context->m_buffer.m_words);
#endif

    // Setup the initial register values
    AL = context->m_chainingVariables[0];            // Left
    BL = context->m_chainingVariables[1];
    CL = context->m_chainingVariables[2];
    DL = context->m_chainingVariables[3];
    EL = context->m_chainingVariables[4];

    AR = context->m_chainingVariables[0];            // Right
    BR = context->m_chainingVariables[1];
    CR = context->m_chainingVariables[2];
    DR = context->m_chainingVariables[3];
    ER = context->m_chainingVariables[4];

    // Round 1
    round = 0;
    for(b = 0; b < 16; ++b) {               // Left
        tempInt = ROL(SL[round][b], AL + F1(BL, CL, DL) + context->m_buffer.m_words[RL[round][b]] + KL[round]) + EL;
        AL = EL;
        EL = DL;
        DL = ROL(10, CL);
        CL = BL;
        BL = tempInt;
    }
    for(b = 0; b < 16; ++b) {              // Right
        tempInt = ROL(SR[round][b], AR + F5(BR, CR, DR) + context->m_buffer.m_words[RR[round][b]] + KR[round]) + ER;
        AR = ER;
        ER = DR;
        DR = ROL(10, CR);
        CR = BR;
        BR = tempInt;
    }

    // Round 2
    ++round;
    for(b = 0; b < 16; ++b) {              // Left
        tempInt = ROL(SL[round][b], AL + F2(BL, CL, DL) + context->m_buffer.m_words[RL[round][b]] + KL[round]) + EL;
        AL = EL;
        EL = DL;
        DL = ROL(10, CL);
        CL = BL;
        BL = tempInt;
    }
    for(b = 0; b < 16; ++b) {               // Right
        tempInt = ROL(SR[round][b], AR + F4(BR, CR, DR) + context->m_buffer.m_words[RR[round][b]] + KR[round]) + ER;
        AR = ER;
        ER = DR;
        DR = ROL(10, CR);
        CR = BR;
        BR = tempInt;
    }

    // Round 3
    ++round;
    for(b = 0; b < 16; ++b) {              // Left
        tempInt = ROL(SL[round][b], AL + F3(BL, CL, DL) + context->m_buffer.m_words[RL[round][b]] + KL[round]) + EL;
        AL = EL;
        EL = DL;
        DL = ROL(10, CL);
        CL = BL;
        BL = tempInt;
    }
    for(b = 0; b < 16; ++b) {               // Right
        tempInt = ROL(SR[round][b], AR + F3(BR, CR, DR) + context->m_buffer.m_words[RR[round][b]] + KR[round]) + ER;
        AR = ER;
        ER = DR;
        DR = ROL(10, CR);
        CR = BR;
        BR = tempInt;
    }

    // Round 4
    ++round;
    for(b = 0; b < 16; ++b) {               // Left
        tempInt = ROL(SL[round][b], AL + F4(BL, CL, DL) + context->m_buffer.m_words[RL[round][b]] + KL[round]) + EL;
        AL = EL;
        EL = DL;
        DL = ROL(10, CL);
        CL = BL;
        BL = tempInt;
    }
    for(b = 0; b < 16; ++b) {               // Right
        tempInt = ROL(SR[round][b], AR + F2(BR, CR, DR) + context->m_buffer.m_words[RR[round][b]] + KR[round]) + ER;
        AR = ER;
        ER = DR;
        DR = ROL(10, CR);
        CR = BR;
        BR = tempInt;
    }

    // Round 5
    ++round;
    for(b = 0; b < 16; ++b) {               // Left
        tempInt = ROL(SL[round][b], AL + F5(BL, CL, DL) + context->m_buffer.m_words[RL[round][b]] + KL[round]) + EL;
        AL = EL;
        EL = DL;
        DL = ROL(10, CL);
        CL = BL;
        BL = tempInt;
    }
    for(b = 0; b < 16; ++b) {               // Right
        tempInt = ROL(SR[round][b], AR + F1(BR, CR, DR) + context->m_buffer.m_words[RR[round][b]] + KR[round]) + ER;
        AR = ER;
        ER = DR;
        DR = ROL(10, CR);
        CR = BR;
        BR = tempInt;
    }

    // Combine results
    tempInt                             = context->m_chainingVariables[1] + CL + DR;
    context->m_chainingVariables[1]     = context->m_chainingVariables[2] + DL + ER;
    context->m_chainingVariables[2]     = context->m_chainingVariables[3] + EL + AR;
    context->m_chainingVariables[3]     = context->m_chainingVariables[4] + AL + BR;
    context->m_chainingVariables[4]     = context->m_chainingVariables[0] + BL + CR;
    context->m_chainingVariables[0]     = tempInt;

    // Clear the buffer
    memset(&(context->m_buffer), 0, sizeof(context->m_buffer));
    context->m_bufferPosition           = 0;
}
