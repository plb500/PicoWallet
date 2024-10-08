#include "seed_utils.h"
#include "hash_utils.h"

#include "platform/wallet_random.h"
#include "big_int/big_int.h"
#include "key_print_utils.h"
#include "cryptography/cifra/pbkdf2.h"
#include "cryptography/cifra/sha2.h"

#include <string.h>

#if DEBUG_SEED_GENERATION
#   include <stdio.h>
#endif


#define MIN_RANDOM_BITS                 (128)
#define MAX_RANDOM_BITS                 (256)

#define SENTENCE_LENGTH_CHARS           ((MNEMONIC_LENGTH * (MAX_MNEMONIC_WORD_LENGTH + 1)) - 1)


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


int16_t get_bip39_word_idx(const char* word) {
    // Stupidly inefficient. I will fix this later
    for(int i = 0; i < 2048; ++i) {
        if(!strcmp(word, BIP39_WORD_LIST[i])) {
            return i;
        }
    }

    return -1;
}

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

int validate_mnemonic(char mnemonics[MNEMONIC_LENGTH][MAX_MNEMONIC_WORD_LENGTH + 1]) {
    // The (24 word) mnemonic sentence encodes 33 bytes of data. 32 for the entropy and 
    //1 for the checksum
    uint8_t encoded[33];
    uint8_t checksum[32];
    int currentByte = 0, currentBit = 7;

    memset(encoded, 0, 33);
    for(int i = 0; i < MNEMONIC_LENGTH; ++i) {
        int16_t idx = get_bip39_word_idx(mnemonics[i]);
        if(idx < 0) {
            return 0;
        } 

        // Index will be 0-2047, encoding 11 bits of data
        for(int j = 10; j >= 0; --j) {
            if(idx & (1 << j)) {
                encoded[currentByte] |= (1 << currentBit);
            }
            --currentBit;
            if(currentBit < 0) {
                ++currentByte;
                currentBit = 7;
            }
        }
    }

    // Hash the recovered bytes to get the checksum
    do_sha256(encoded, 32, checksum);

    return (checksum[0] == encoded[32]);
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
    printf("Entropy bytes:\n");
    print_bytes(ctx->entropy, 64, 1);
    printf("\n");
#endif
    apply_entropy_checksum(ctx->entropy, ENTROPY_BITS);


    // Step 2 - convert to mnemonic wordlist
    entropy_to_mnemonic(ctx->entropy, ENTROPY_BITS, ctx->mnemonic);

#if DEBUG_SEED_GENERATION
#define WORD_SPACE      (10)
    printf("Mnemonic phrase:");
    printf("\n    ");
    for(int i = 0; i < MNEMONIC_LENGTH; ++i) {

        printf("%s", ctx->mnemonic[i]);
        if(((i+1) % 4) == 0) {
            printf("\n    ");
        } else {
            uint8_t wordLen = strlen(ctx->mnemonic[i]);
            uint8_t numSpaces = (WORD_SPACE - wordLen);
            for(int i = 0; i < numSpaces; ++i) {
                printf(" ");
            }
        }
    }
    printf("\n");
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
        ctx->mnemonic, MNEMONIC_LENGTH,
        _mnemonicPassphraseBuffer, mpLen,
        ctx->seed
    );

#if DEBUG_SEED_GENERATION
    printf("Seed:\n");
    print_bytes(ctx->seed, EXTENDED_MASTER_KEY_LENGTH, 1);
    printf("\n");

    printf("Private Key: 0x");
    for(int i = 0; i < 32; ++i) {
        printf("%02X", ctx->seed[i]);
    }
    printf("\n");

    printf(" Chain code: 0x");
    for(int i = 32; i < 64; ++i) {
        printf("%02X", ctx->seed[i]);
    }
    printf("\n\n               ****************\n\n");

#endif
}
