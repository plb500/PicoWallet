#include "seed_utils.h"
#include "hash_utils.h"

#include "platform/wallet_random.h"
#include "big_int/big_int.h"
#include "cryptography/cifra/pbkdf2.h"
#include "cryptography/cifra/sha2.h"

#include <string.h>

#if DEBUG_SEED_GENERATION
#   include <stdio.h>
#endif


#define MIN_RANDOM_BITS                 (128)
#define MAX_RANDOM_BITS                 (256)

#define BIP39_MAX_WORD_LENGTH           (8)
#define SENTENCE_LENGTH_CHARS           ((NUM_WORDS_IN_MNEMONIC_SENTENCE * (BIP39_MAX_WORD_LENGTH + 1)) - 1)


#if USE_DEBUG_ENTROPY
uint8_t DEBUG_ENTROPY_BYTES[] = {
        0x09, 0x11, 0xc4, 0x0d, 0xfe, 0x40, 0xc7, 0x17, 0x7e, 0xf8, 0xbd, 0x72, 0xd0, 0x3c, 0x33, 0xd9, 
        0x97, 0xba, 0x98, 0xeb, 0x2b, 0x12, 0x0f, 0x7d, 0x70, 0x6d, 0x67, 0x2d, 0xea, 0x79, 0x40, 0x1e,
        0x00, 0x00, 0x00, 0x00, 0x00
    };
#endif

const char* MNEMONIC_PREFIX             = "mnemonic";
#define MNEMONIC_PREFIX_LENGTH          (8)

extern const char* BIP39_WORD_LIST[];
uint8_t _bipSentenceBuffer[SENTENCE_LENGTH_CHARS];
uint8_t _mnemonicPassphraseBuffer[MNEMONIC_PREFIX_LENGTH + MAX_MNEMONIC_PASSPHRASE_LENGTH];


int create_random_bits(int numBits, uint8_t* dest) {
    if(numBits < MIN_RANDOM_BITS) {
        numBits = MIN_RANDOM_BITS- 1;
    } else if(numBits > MAX_RANDOM_BITS) {
        numBits = MAX_RANDOM_BITS;
    }

    // Ensure number of bits is a multiple of 32
    if((numBits & 31) != 0) {
        numBits = (numBits | 31) + 1;
    }

    int numBytes = (numBits >> 3);

    for(int i = 0; i < numBytes; ++i) {
        dest[i] = get_random_byte();
    }

    return numBits;
}

int apply_entropy_checksum(uint8_t *data, int numBits) {
    int numChecksumBits = (numBits >> 5);
    int numChecksumBytes = (numChecksumBits >> 3) + 1;
    int byteCount = (numBits >> 3);
    
    uint8_t shaBytes[64];
    do_sha256(data, byteCount, shaBytes);

    // Since we stop on an exact boundary anyway there's no point trying to specifically copy
    // only the bits we need, so just add the whole bytes to save some time
    for(int i = 0; i < numChecksumBytes; ++i) {
        data[byteCount + i] = shaBytes[i];
    }

    return numChecksumBits;
}

int create_entropy(uint8_t* entropy, int numBits) {
    int totalEntropyBits = create_random_bits(numBits, entropy);
    apply_entropy_checksum(entropy, totalEntropyBits);
}

int entropy_to_mnemonic(uint8_t *entropy, int numEntropyBits, const char** sentence) {
    // The incoming entropy bits are split into blocks of 11 bits, each block points to one of
    // the 2048 entries in the BIP-39 word list

    int numWords = (numEntropyBits / 11) + 1;
    for(int bit = 0, word = 0; bit < numEntropyBits; bit += 11, ++word) {
        uint16_t wordIndex = 0;
        for(int srcBit = (bit + 10), destBit = 0; destBit < 11; srcBit--, destBit++) {
            int srcByteIndex = (srcBit >> 3);
            int srcBitIndex = (((srcByteIndex + 1) << 3) - srcBit) - 1;
            uint16_t bitVal = (entropy[srcByteIndex] & (1 << srcBitIndex)) >> srcBitIndex; 
            wordIndex |= (bitVal << destBit);
        }
        sentence[word] = BIP39_WORD_LIST[wordIndex];
    }

    return numWords;
}

int mnemonic_to_seed(
    const char** mnemonic, int numWords,
    const char* passphrase, int passphraseLen,
    uint8_t* seed
) {
    // Concatenate words into string
    memset(_bipSentenceBuffer, ' ', SENTENCE_LENGTH_CHARS);
    int sentenceLen = 0;
    for(int i = 0; i < numWords; ++i) {
        char* writePtr = _bipSentenceBuffer + sentenceLen;
        size_t wordLen = strlen(mnemonic[i]);
        memcpy(writePtr, mnemonic[i], wordLen);
        sentenceLen += (wordLen + 1);
    }
    --sentenceLen;

    // Hash mnemonic (+ passphrase) to get seed
    cf_pbkdf2_hmac(
        _bipSentenceBuffer, sentenceLen, 
        passphrase, passphraseLen, 
        2048, 
        seed, EXTENDED_MASTER_KEY_LENGTH,
        &cf_sha512
    );

    return EXTENDED_MASTER_KEY_LENGTH;
}

int generate_seed(SeedCtx* ctx, const char* passphrase, int passphraseLen) {
    // Step 1 - generate entropy and apply checksum
#if USE_DEBUG_ENTROPY
    memcpy(ctx->entropy, DEBUG_ENTROPY_BYTES, (ENTROPY_BITS / 8));
#else
    create_random_bits(ENTROPY_BITS, ctx->entropy);
#endif

#if DEBUG_SEED_GENERATION
    printf("Entropy bytes: ");
    print_bytes(ctx->entropy, 64, 0);
    printf("\n\n");
#endif
    apply_entropy_checksum(ctx->entropy, ENTROPY_BITS);


    // Step 2 - convert to mnemonic wordlist
    entropy_to_mnemonic(ctx->entropy, ENTROPY_BITS, ctx->mnemonic);

#if DEBUG_SEED_GENERATION
    printf("Mnemonic phrase:");
    for(int i = 0; i < NUM_WORDS_IN_MNEMONIC_SENTENCE; ++i) {
        if((i % 4) == 0) {
            printf("\n    ");
        } else {
            printf(" ");
        }

        printf("%s", ctx->mnemonic[i]);
    }
    printf("\n\n");
#endif


    // Step 3 - convert to seed
    memcpy(_mnemonicPassphraseBuffer, MNEMONIC_PREFIX, MNEMONIC_PREFIX_LENGTH);
    int mpLen = MNEMONIC_PREFIX_LENGTH;
    if(passphrase && passphraseLen) {
        int cappedPassphraseLen = MIN(passphraseLen, MAX_MNEMONIC_PASSPHRASE_LENGTH);
        memcpy((_mnemonicPassphraseBuffer + MNEMONIC_PREFIX_LENGTH), passphrase, cappedPassphraseLen);
        mpLen += cappedPassphraseLen;
    }

    mnemonic_to_seed(
        ctx->mnemonic, NUM_WORDS_IN_MNEMONIC_SENTENCE,
        _mnemonicPassphraseBuffer, mpLen,
        ctx->seed
    );

#if DEBUG_SEED_GENERATION
    printf("Seed: ");
    print_bytes(ctx->seed, EXTENDED_MASTER_KEY_LENGTH, 0);
    printf("\n\n");

    printf("Private Key:");
    for(int i = 0; i < 32; ++i) {
        printf("%02X", ctx->seed[i]);
    }
    printf("\n\n");

    printf("Chain code:");
    for(int i = 32; i < 64; ++i) {
        printf("%02X", ctx->seed[i]);
    }
    printf("\n        ********\n\n");

#endif
}
