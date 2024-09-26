#ifndef _SEED_UTILS_H_
#define _SEED_UTILS_H_

#include <stdint.h>


#define EXTENDED_MASTER_KEY_LENGTH          (64)
#define ENTROPY_BITS                        (256)
#define ENTROPY_CHECKSUM_BYTES              (1)
#define MAX_MNEMONIC_PASSPHRASE_LENGTH      (16)
#define MNEMONIC_LENGTH                     (24)
#define MAX_MNEMONIC_WORD_LENGTH            (8)


typedef struct {
    uint8_t entropy[(ENTROPY_BITS / 8) + ENTROPY_CHECKSUM_BYTES];
    const char* mnemonic[MNEMONIC_LENGTH];
    uint8_t seed[EXTENDED_MASTER_KEY_LENGTH];
} SeedCtx;


int generate_seed(SeedCtx* ctx, const char* passphrase, int passphraseLen);

int create_entropy(uint8_t* entropy, int numBits);
int apply_entropy_checksum(uint8_t *data, int numBits);
int entropy_to_mnemonic(uint8_t *entropy, int numEntropyBits, const char** sentence);
int mnemonic_to_seed(
    const char** mnemonic, int numWords,
    const char* passphrase, int passphraseLen,
    uint8_t* seed
);


#endif      // _SEED_UTILS_H_
