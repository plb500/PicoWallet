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
//  | Version Code (unencrypted)    |   1 byte major, 1 byte minor
//  | 2 bytes                       |
//  +-------------------------------+
//  | Verification Header           |   PKBKDF-SHA256 of user password
//  | 32 bytes                      |
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
//  |              Total: 314 bytes |
//  | Padded to multiple            |
//  |         of keysize: 320 bytes |
//  +-------------------------------+
//
//
#define WALLET_VERSION_MAJOR   (0x00)
#define WALLET_VERSION_MINOR   (0x01)
const uint16_t WALLET_VERSION  = ((WALLET_VERSION_MAJOR << 8) | WALLET_VERSION_MINOR);


#define PASSWORD_BLOCK_LENGTH   (16)
#define BASE_KEY_INDEX          (44)


// Work variables
static cf_aes_context aesContext;
static uint8_t walletSerializationBuffer[SERIALIZED_WALLET_SIZE];


// Bulk out the user password (8 chars) to the AES key size (16 chars)
void pad_password(const char* passwordSrc, char* passwordDst) {
    for(int dst = 0, src = 0; dst < PASSWORD_BLOCK_LENGTH; ++dst) {
        passwordDst[dst] = passwordSrc[src];
        if(++src >= USER_PASSWORD_LENGTH) {
            src = 0;
        }
    }
}

int serialize_wallet(const HDWallet* wallet, uint8_t* dest) {
    uint8_t* writePtr = dest;
    uint8_t padBytes;

    // Write version code
    memcpy(dest, &WALLET_VERSION, sizeof(WALLET_VERSION));
    writePtr += sizeof(WALLET_VERSION);

    // Write password hash header
    cf_pbkdf2_hmac(
        wallet->password, USER_PASSWORD_LENGTH, 
        PASSCODE_PEPPER, strlen(PASSCODE_PEPPER), 
        2048, 
        writePtr, PBKDF2_HMAC_SHA256_SIZE,
        &cf_sha256
    );
    writePtr += PBKDF2_HMAC_SHA256_SIZE;

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
    padBytes = PASSWORD_BLOCK_LENGTH- ((writePtr - dest) % PASSWORD_BLOCK_LENGTH);
    for(int i = 0; i < padBytes; ++i) {
        *writePtr++ = 0xFF;
    }

    return (writePtr - dest);
}

wallet_error deserialize_wallet(const uint8_t* src, HDWallet* wallet) {
    const uint8_t* readPtr = src;
    uint16_t version;
    uint8_t workBuffer[64];
    const uECC_Curve curve = uECC_secp256k1();

    // Read and validate version code
    memcpy(&version, readPtr, sizeof(version));
    readPtr += sizeof(version);
    if(version != WALLET_VERSION) {
        return WALLET_ERROR(WF_FILE_VERSION_MISMATCH, 0);
    }

    // Validate password hash header
    cf_pbkdf2_hmac(
        wallet->password, USER_PASSWORD_LENGTH, 
        PASSCODE_PEPPER, strlen(PASSCODE_PEPPER), 
        2048, 
        workBuffer, PBKDF2_HMAC_SHA256_SIZE,
        &cf_sha256
    );
    if(memcmp(workBuffer, readPtr, PBKDF2_HMAC_SHA256_SIZE) != 0) {
        return WALLET_ERROR(WF_INVALID_PASSWORD, 0);
    }
    readPtr += PBKDF2_HMAC_SHA256_SIZE;

    // Read private key
    memcpy(wallet->masterKey.privateKey, readPtr, PRIVATE_KEY_LENGTH);
    readPtr += PRIVATE_KEY_LENGTH;

    // Read chain code
    memcpy(wallet->masterKey.chainCode, readPtr, CHAIN_CODE_LENGTH); 
    readPtr += CHAIN_CODE_LENGTH;

    // Get and compress the public key
    int success = uECC_compute_public_key(wallet->masterKey.privateKey, workBuffer, curve);
    if(success) {
        wallet->masterKey.publicKey[0] = (workBuffer[UNCOMPRESSED_PUBLIC_KEY_LENGTH - 1] & 1) ? 0x03 : 0x02;
        memcpy(&(wallet->masterKey.publicKey[1]), workBuffer, (PUBLIC_KEY_LENGTH - 1));
    }

    // Read mnemonic
    for(int i = 0; i < MNEMONIC_LENGTH; ++i) {
        strncpy(wallet->mnemonicSentence[i], readPtr, MAX_MNEMONIC_WORD_LENGTH + 1);
        readPtr += (MAX_MNEMONIC_WORD_LENGTH + 1);
    }

    return NO_ERROR;
}


int init_new_wallet(HDWallet* wallet, const uint8_t* password, const uint8_t* mnemonic, int mnemonicLen) {
    generate_master_key(mnemonic, mnemonicLen, &wallet->masterKey, wallet->mnemonicSentence);
    set_wallet_password(wallet, password);
    derive_child_key(&wallet->masterKey, BASE_KEY_INDEX, &wallet->baseKey44);
    return 1;
}

wallet_error decrypt_wallet_data(uint8_t* data, HDWallet* dest) {
    uint8_t aesBlock[AES_BLOCKSZ];
    uint8_t paddedPassword[PASSWORD_BLOCK_LENGTH];
    uint8_t* decryptPtr = data;
    wallet_error deserializeResult;
    int decryptCount = 0;

    // Decrypt the wallet bytes
    pad_password(dest->password, paddedPassword);
    cf_aes_init(&aesContext, paddedPassword, PASSWORD_BLOCK_LENGTH);
    while(decryptCount < SERIALIZED_WALLET_SIZE) {
        cf_aes_decrypt(&aesContext, decryptPtr, aesBlock);
        memcpy(decryptPtr, aesBlock, AES_BLOCKSZ);
        decryptPtr += AES_BLOCKSZ;
        decryptCount += AES_BLOCKSZ;
    }
    cf_aes_finish(&aesContext);

    // Deserialize decrypted bytes into usable wallet
    deserializeResult = deserialize_wallet(data, dest);
    if(deserializeResult != NO_ERROR) {
        return deserializeResult;
    }

    derive_child_key(&dest->masterKey, BASE_KEY_INDEX, &dest->baseKey44);

    return NO_ERROR;
}

wallet_error recover_wallet(HDWallet* wallet) {
    wallet_error readMnemonicResult;
    char mnemonics[MNEMONIC_LENGTH][MAX_MNEMONIC_WORD_LENGTH + 1];

    readMnemonicResult = read_mnemonics_from_disk(wallet->mnemonicSentence);
    if(readMnemonicResult != NO_ERROR) {
        return readMnemonicResult;
    }


    // Build keys
    generate_master_key_from_mnemonic(wallet->mnemonicSentence, &wallet->masterKey);
    derive_child_key(&wallet->masterKey, BASE_KEY_INDEX, &wallet->baseKey44);

    return NO_ERROR;
}

wallet_error save_wallet(const HDWallet* wallet) {
    uint8_t aesBlock[AES_BLOCKSZ];
    uint8_t paddedPassword[PASSWORD_BLOCK_LENGTH];
    uint8_t* encryptPtr = walletSerializationBuffer;
    int encryptCount = 0;

    // Serialize wallet to raw bytes
    serialize_wallet(wallet, walletSerializationBuffer);

    // Encrypt
    pad_password(wallet->password, paddedPassword);
    cf_aes_init(&aesContext, paddedPassword, PASSWORD_BLOCK_LENGTH);
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
