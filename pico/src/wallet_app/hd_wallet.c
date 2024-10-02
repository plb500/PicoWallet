#include "hd_wallet.h"

#include "cryptography/cifra/pbkdf2.h"
#include "cryptography/cifra/sha2.h"
#include "cryptography/cifra/aes.h"
#include "cryptography/uECC/uECC.h"

#include "utils/hash_utils.h"
#include "utils/wallet_file.h"

#include <string.h>
#include <stdio.h>


//
//      Wallet serialization format
//  
//  +-------------------------------+
//  | Version Code                  |   1 byte major, 1 byte minor
//  | 2 bytes                       |
//  +-------------------------------+
//  | Verification Header           |   Last 16 bytes of PKBKDF-SHA256 of user password
//  | 16 bytes                      |
//  +-------------------------------+
//  | Master Private Key            |
//  | 32 bytes                      |
//  +-------------------------------+
//  | Master Chain Code             |
//  | 32 bytes                      |
//  +-------------------------------+
//  | Mnemonic Sentence             |   Not each word is going to use the full 8 chars
//  | 216 bytes ((8+1) * 24)        |   Smaller words are zero padded
//  +-------------------------------+
//  |              Total: 298 bytes |
//  | Padded to multiple            |
//  |      of block size: 304 bytes |
//  +-------------------------------+
//
//
#define WALLET_VERSION_MAJOR   (0x00)
#define WALLET_VERSION_MINOR   (0x01)
const uint16_t WALLET_VERSION  = ((WALLET_VERSION_MAJOR << 8) | WALLET_VERSION_MINOR);


#define PASSWORD_BLOCK_LENGTH   (16)
#define VALIDATION_BYTES_LENGTH (PBKDF2_HMAC_SHA256_SIZE - PASSWORD_BLOCK_LENGTH)
#define BASE_KEY_INDEX          (44)


// Work variables
static cf_aes_context aesContext;
static uint8_t walletSerializationBuffer[SERIALIZED_WALLET_SIZE];


int serialize_wallet(const HDWallet* wallet, uint8_t* dest, uint8_t* validationBytes) {
    uint8_t* writePtr = dest;
    uint8_t padBytes;

    // Write version code
    memcpy(dest, &WALLET_VERSION, sizeof(WALLET_VERSION));
    writePtr += sizeof(WALLET_VERSION);

    // Write validation bytes
    memcpy(writePtr, validationBytes, VALIDATION_BYTES_LENGTH);
    writePtr += VALIDATION_BYTES_LENGTH;

    // Write private key
    memcpy(writePtr, wallet->masterKey.privateKey, PRIVATE_KEY_LENGTH);
    writePtr += PRIVATE_KEY_LENGTH;

    // Write chain code
    memcpy(writePtr, wallet->masterKey.chainCode, CHAIN_CODE_LENGTH); 
    writePtr += CHAIN_CODE_LENGTH;

    // Write mnemonic
    for(int i = 0; i < MNEMONIC_LENGTH; ++i) {
        strncpy(writePtr, wallet->mnemonicSentence[i], MAX_MNEMONIC_WORD_LENGTH + 1);
        writePtr += (MAX_MNEMONIC_WORD_LENGTH + 1);
    }

    // Pad to a multiple of password block length
    padBytes = AES_BLOCKSZ- ((writePtr - dest) % AES_BLOCKSZ);
    for(int i = 0; i < padBytes; ++i) {
        *writePtr++ = 0xFF;
    }

    return (writePtr - dest);
}

wallet_error deserialize_wallet(const uint8_t* src, HDWallet* wallet, uint8_t* validationBytes) {
    const uint16_t* versionPtr = (uint16_t*) src;
    const uint8_t* passwordHashPtr = (src + sizeof(uint16_t));
    const uint8_t* privateKeyPtr = (passwordHashPtr + VALIDATION_BYTES_LENGTH);
    const uint8_t* chainCodePtr = (privateKeyPtr + PRIVATE_KEY_LENGTH);
    const uint8_t* mnemonicPtr =  (chainCodePtr + CHAIN_CODE_LENGTH);

    uint8_t workBuffer[64];
    const uECC_Curve curve = uECC_secp256k1();

    // Validate password hash header
    if(memcmp(passwordHashPtr, validationBytes, VALIDATION_BYTES_LENGTH) != 0) {
        return WALLET_ERROR(WF_INVALID_PASSWORD, 0);
    }

    // Read and validate version code
    if(*versionPtr != WALLET_VERSION) {
        return WALLET_ERROR(WF_FILE_VERSION_MISMATCH, 0);
    }

    // Read private key
    memcpy(wallet->masterKey.privateKey, privateKeyPtr, PRIVATE_KEY_LENGTH);

    // Read chain code
    memcpy(wallet->masterKey.chainCode, chainCodePtr, CHAIN_CODE_LENGTH); 

    // Get and compress the public key
    int success = uECC_compute_public_key(wallet->masterKey.privateKey, workBuffer, curve);
    if(success) {
        wallet->masterKey.publicKey[0] = (workBuffer[UNCOMPRESSED_PUBLIC_KEY_LENGTH - 1] & 1) ? 0x03 : 0x02;
        memcpy(&(wallet->masterKey.publicKey[1]), workBuffer, (PUBLIC_KEY_LENGTH - 1));
    }

    // Read mnemonic
    for(int i = 0; i < MNEMONIC_LENGTH; ++i) {
        strncpy(wallet->mnemonicSentence[i], mnemonicPtr, MAX_MNEMONIC_WORD_LENGTH + 1);
        mnemonicPtr += (MAX_MNEMONIC_WORD_LENGTH + 1);
    }

    return NO_ERROR;
}


int init_new_wallet(HDWallet* wallet, const uint8_t* password, const uint8_t* mnemonic, int mnemonicLen) {
    generate_master_key(mnemonic, mnemonicLen, &wallet->masterKey, wallet->mnemonicSentence);
    set_wallet_password(wallet, password);
    derive_child_key(&wallet->masterKey, BASE_KEY_INDEX, true, &wallet->baseKey44);
    return 1;
}

wallet_error decrypt_wallet_data(uint8_t* data, HDWallet* dest) {
    uint8_t passwordHash[PBKDF2_HMAC_SHA256_SIZE];
    uint8_t* dataPtr = data;
    uint8_t* decryptedDataPtr = walletSerializationBuffer;
    wallet_error deserializeResult;
    int decryptCount = 0;

    // Get password hash
    cf_pbkdf2_hmac(
        dest->password, USER_PASSWORD_LENGTH, 
        PASSCODE_SALT, strlen(PASSCODE_SALT), 
        2048, 
        passwordHash, PBKDF2_HMAC_SHA256_SIZE,
        &cf_sha256
    );

    // Decrypt the wallet bytes
    cf_aes_init(&aesContext, passwordHash, PBKDF2_HMAC_SHA256_SIZE);
    while(decryptCount < SERIALIZED_WALLET_SIZE) {
        cf_aes_decrypt(&aesContext, dataPtr, decryptedDataPtr);
        dataPtr += AES_BLOCKSZ;
        decryptedDataPtr += AES_BLOCKSZ;
        decryptCount += AES_BLOCKSZ;
    }
    cf_aes_finish(&aesContext);

    // Deserialize decrypted bytes into usable wallet
    deserializeResult = deserialize_wallet(walletSerializationBuffer, dest, (passwordHash + PASSWORD_BLOCK_LENGTH));
    if(deserializeResult != NO_ERROR) {
        return deserializeResult;
    }

    // Get the BIP44 m/44' base key
    derive_child_key(&dest->masterKey, BASE_KEY_INDEX, true, &dest->baseKey44);

    return NO_ERROR;
}

wallet_error recover_wallet(HDWallet* wallet) {
    wallet_error readMnemonicResult;
    char mnemonics[MNEMONIC_LENGTH][MAX_MNEMONIC_WORD_LENGTH + 1];

    readMnemonicResult = read_mnemonics_from_disk(wallet->mnemonicSentence);
    if(readMnemonicResult != NO_ERROR) {
        return readMnemonicResult;
    }

    // Make sure the mnemonic is valid
    if(!validate_mnemonic(wallet->mnemonicSentence)) {
        return WALLET_ERROR(WF_MNEMONIC_CHECKSUM_INVALID, 0);
    }

    // Build keys
    generate_master_key_from_mnemonic(wallet->mnemonicSentence, &wallet->masterKey);
    derive_child_key(&wallet->masterKey, BASE_KEY_INDEX, true, &wallet->baseKey44);

    return NO_ERROR;
}

wallet_error save_wallet(const HDWallet* wallet) {
    uint8_t passwordHash[PBKDF2_HMAC_SHA256_SIZE];
    uint8_t aesBlock[AES_BLOCKSZ];
    uint8_t paddedPassword[PASSWORD_BLOCK_LENGTH];
    uint8_t* encryptPtr = walletSerializationBuffer;
    int encryptCount = 0;

    // Get password hash
    cf_pbkdf2_hmac(
        wallet->password, USER_PASSWORD_LENGTH, 
        PASSCODE_SALT, strlen(PASSCODE_SALT), 
        2048, 
        passwordHash, PBKDF2_HMAC_SHA256_SIZE,
        &cf_sha256
    );

    // Serialize wallet to raw bytes
    serialize_wallet(wallet, walletSerializationBuffer, (passwordHash + PASSWORD_BLOCK_LENGTH));

    // Encrypt
    cf_aes_init(&aesContext, passwordHash, PBKDF2_HMAC_SHA256_SIZE);
    while(encryptCount < SERIALIZED_WALLET_SIZE) {
        cf_aes_encrypt(&aesContext, encryptPtr, aesBlock);
        memcpy(encryptPtr, aesBlock, AES_BLOCKSZ);
        encryptCount += AES_BLOCKSZ;
        encryptPtr += AES_BLOCKSZ;
    }
    cf_aes_finish(&aesContext);

    // Save to disk
    return save_wallet_data_to_disk(walletSerializationBuffer);
}

void set_wallet_password(HDWallet* wallet, const uint8_t* password) {
    if(password) {
        memcpy(wallet->password, password, USER_PASSWORD_LENGTH);
    }
}
