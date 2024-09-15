#include "wallet_file.h"

#include "utils/big_int/big_int.h"
#include "utils/seed_utils.h"
#include "utils/hash_utils.h"
#include "cryptography/uECC/uECC.h"
#include "encoding/base58.h"
#include "encoding/bech32.h"

#include "cryptography/cifra/hmac.h"
#include "cryptography/cifra/sha2.h"

#include <string.h>
#include <stdio.h>


#define ADDRESS_VERSION_FIELD_LENGTH        (4)
#define DEPTH_VERSION_FIELD_LENGTH          (1)
#define PARENT_FINGERPRINT_FIELD_LENGTH     (4)
#define CHILD_NUMBER_FIELD_LENGTH           (4)
#define CHAIN_CODE_FIELD_LENGTH             (32)
#define KEY_FIELD_LENGTH                    (33)
#define CHECKSUM_FIELD_LENGTH               (4)
#define ADDRESS_SERIALIZATION_LENGTH        (       \
    ADDRESS_VERSION_FIELD_LENGTH +                  \
    DEPTH_VERSION_FIELD_LENGTH +                    \
    PARENT_FINGERPRINT_FIELD_LENGTH +               \
    CHILD_NUMBER_FIELD_LENGTH +                     \
    CHAIN_CODE_FIELD_LENGTH +                       \
    KEY_FIELD_LENGTH +                              \
    CHECKSUM_FIELD_LENGTH                           \
)

#define HARDENED_CHILD_INDEX_OFFSET         (0x80000000)

const uint8_t SECP256K1_CURVE_ORDER[32] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE,
    0xBA, 0xAE, 0xDC, 0xE6, 0xAF, 0x48, 0xA0, 0x3B,
    0xBF, 0xD2, 0x5E, 0x8C, 0xD0, 0x36, 0x41, 0x41
};

const uint8_t PUBLIC_KEY_ADDRESS_PREFIX[]   = {0x04, 0x88, 0xB2, 0x1E};
const uint8_t PRIVATE_KEY_ADDRESS_PREFIX[]  = {0x04, 0x88, 0xAD, 0xE4};
#define KEY_ADDRESS_PREFIX_SIZE (4)


// Pre-allocated work buffer
#define WORK_BUFFER_SIZE        (512)
uint8_t _workBuffer[WORK_BUFFER_SIZE];


int generate_master_key(const uint8_t *mnemonic, int mnemonicLen, ExtendedKey* dest) {
    SeedCtx* ctx = (SeedCtx*) _workBuffer;
    uint8_t* key = _workBuffer + sizeof(SeedCtx);
    const uECC_Curve curve = uECC_secp256k1();

    // Set master key base variables
    dest->depth = 0;
    dest->index = 0;
    memset(dest->parentFingerprint, 0, FINGERPRINT_LENGTH);

    // Generate our random seed
    generate_seed(ctx, mnemonic, mnemonicLen);

    // Run the seed through HMAC-SHA512 to get our master extended private key and chain code
    cf_hmac(
        "Bitcoin seed", 12, 
        ctx->seed, EXTENDED_MASTER_KEY_LENGTH,
        key,
        &cf_sha512
    );

    memcpy(dest->privateKey, key, PRIVATE_KEY_LENGTH); 
    memcpy(dest->chainCode, key + PRIVATE_KEY_LENGTH, CHAIN_CODE_LENGTH); 

    // Get and compress the public key
    int success = uECC_compute_public_key(dest->privateKey, key, curve);
    if(success) {
        dest->publicKey[0] = (key[63] & 1) ? 0x03 : 0x02;
        memcpy(&(dest->publicKey[1]), key, (PUBLIC_KEY_LENGTH - 1));
    }

    // Get fingerprint
    hash_160(dest->publicKey, PUBLIC_KEY_LENGTH, _workBuffer);
    memcpy(dest->fingerprint, _workBuffer, FINGERPRINT_LENGTH);
}

int derive_child_key(const ExtendedKey* parentKey, uint32_t index, ExtendedKey* dest) {
    const uECC_Curve curve = uECC_secp256k1();
    uint8_t* hmacData = _workBuffer;
    uint8_t* workBuffer = _workBuffer + PRIVATE_KEY_LENGTH + 4 + 1;
    
    index += HARDENED_CHILD_INDEX_OFFSET;

    memcpy(dest->parentFingerprint, parentKey->fingerprint, FINGERPRINT_LENGTH);
    dest->depth = (parentKey->depth + 1);
    dest->index = index;

    // Generate new key
    hmacData[0] = 0;
    memcpy(&(hmacData[1]), parentKey->privateKey, PRIVATE_KEY_LENGTH);
    hmacData[PRIVATE_KEY_LENGTH + 1] = ((uint8_t*) &index)[3];
    hmacData[PRIVATE_KEY_LENGTH + 2] = ((uint8_t*) &index)[2];
    hmacData[PRIVATE_KEY_LENGTH + 3] = ((uint8_t*) &index)[1];
    hmacData[PRIVATE_KEY_LENGTH + 4] = ((uint8_t*) &index)[0];

    // Run the parent key parameters through HMAC-SHA512 to get our master extended private key and chain code
    cf_hmac(
        parentKey->chainCode, CHAIN_CODE_LENGTH, 
        hmacData, PRIVATE_KEY_LENGTH + 5,
        workBuffer,
        &cf_sha512
    );

    memcpy(dest->privateKey, workBuffer, PRIVATE_KEY_LENGTH); 
    memcpy(dest->chainCode, workBuffer + PRIVATE_KEY_LENGTH, CHAIN_CODE_LENGTH); 

    // Output will be 33 bytes because of potential overflow
    int overflow = bytewise_add(dest->privateKey, PRIVATE_KEY_LENGTH, parentKey->privateKey, PRIVATE_KEY_LENGTH, workBuffer);
    insert_and_shift(workBuffer, PRIVATE_KEY_LENGTH, overflow);
    memcpy(dest->privateKey, workBuffer, PRIVATE_KEY_LENGTH + 1); 

    // After the mod the MSB will have been cleared so we're back to 32 bytes
    bytewise_mod(dest->privateKey, 33, SECP256K1_CURVE_ORDER, 32, workBuffer);
    memcpy(dest->privateKey, workBuffer + 1, PRIVATE_KEY_LENGTH); 

    // Get and compress the public key
    int success = uECC_compute_public_key(dest->privateKey, workBuffer, curve);
    if(success) {
        dest->publicKey[0] = (workBuffer[63] & 1) ? 0x03 : 0x02;
        memcpy(&(dest->publicKey[1]), workBuffer, (PUBLIC_KEY_LENGTH -  1));
    }

    // Get fingerprint
    hash_160(dest->publicKey, PUBLIC_KEY_LENGTH, _workBuffer);
    memcpy(dest->fingerprint, _workBuffer, FINGERPRINT_LENGTH);
}

int init_new_wallet(HDWallet* wallet, const uint8_t* mnemonic, int mnemonicLen) {
    generate_master_key(mnemonic, mnemonicLen, &wallet->masterKey);

    return 1;
}

int get_extended_key_address(const ExtendedKey* key, uint8_t* address, int public) {
    uint8_t* writePtr = _workBuffer;
    uint8_t* hash = _workBuffer + ADDRESS_SERIALIZATION_LENGTH;

    // Version
    if(public) {
        memcpy(writePtr, PUBLIC_KEY_ADDRESS_PREFIX, KEY_ADDRESS_PREFIX_SIZE);
        writePtr += KEY_ADDRESS_PREFIX_SIZE;

    } else {
        memcpy(writePtr, PRIVATE_KEY_ADDRESS_PREFIX, KEY_ADDRESS_PREFIX_SIZE);
        writePtr += KEY_ADDRESS_PREFIX_SIZE;
    }

    // Depth (0 for master)
    *writePtr++ = key->depth;

    // Fingerprint (00000000 for master)
    memcpy(writePtr, key->fingerprint, FINGERPRINT_LENGTH);
    writePtr += FINGERPRINT_LENGTH;

    // Child number (00000000 for master)
    // TODO: Endianness?
    memset(writePtr, 0, 4);
    writePtr += 4;

    // Chain code
    memcpy(writePtr, key->chainCode, CHAIN_CODE_LENGTH);
    writePtr += CHAIN_CODE_LENGTH;

    // Key
    if(public) {
        memcpy(writePtr, key->publicKey, PUBLIC_KEY_LENGTH);
        writePtr += PUBLIC_KEY_LENGTH;
    } else {
        *writePtr++ = 0;
        memcpy(writePtr, key->privateKey, PRIVATE_KEY_LENGTH);
        writePtr += PRIVATE_KEY_LENGTH;
    }

    // Checksum
    double_256(_workBuffer, (ADDRESS_SERIALIZATION_LENGTH - CHECKSUM_FIELD_LENGTH), hash);
    memcpy(writePtr, hash, CHECKSUM_FIELD_LENGTH);
    writePtr += CHECKSUM_FIELD_LENGTH;

    // Base58
    return base58_encode(_workBuffer, ADDRESS_SERIALIZATION_LENGTH, address);
}

int get_extended_private_key_address(const ExtendedKey* key, uint8_t* address) {
    return get_extended_key_address(key, address, 0);
}

int get_extended_public_key_address(const ExtendedKey* key, uint8_t* address) {
    return get_extended_key_address(key, address, 1);
}

int get_p2pkh_public_address(const ExtendedKey* key, uint8_t* address) {
    uint8_t* prefix = _workBuffer;
    uint8_t* hash160 = _workBuffer + 1;
    uint8_t* sha256 = hash160 + 20;

    *prefix = 0x00;
    hash_160(key->publicKey, PUBLIC_KEY_LENGTH, hash160);
    double_256(prefix, 21, sha256);
    base58_encode(prefix, 25, address);

    return 34;
}

int get_p2wpkh_public_address(const ExtendedKey* key, uint8_t* address) {
    const char* hrp = "bc";
    uint8_t* hash160 = _workBuffer;

    hash_160(key->publicKey, PUBLIC_KEY_LENGTH, hash160);

    segwit_addr_encode(
        address,
        hrp,
        0x00,
        hash160, 20
    );

    return 42;
}

int get_private_key_wif(const ExtendedKey* key, BTCNetwork network, uint8_t* address) {
    uint8_t* prefix = _workBuffer;
    uint8_t* privateKey = _workBuffer + 1;
    uint8_t* compression = privateKey + PRIVATE_KEY_LENGTH;
    uint8_t* sha256 = compression + 1;

    *prefix = 0x80;
    memcpy(privateKey, key->privateKey, PRIVATE_KEY_LENGTH);
    *compression = 0x01;

    double_256(prefix, PRIVATE_KEY_LENGTH + 1 + 1, sha256);

    return base58_encode(prefix, 2 + PRIVATE_KEY_LENGTH + CHECKSUM_FIELD_LENGTH, address);
}
