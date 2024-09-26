#include "hd_wallet.h"

#include "cryptography/cifra/pbkdf2.h"
#include "cryptography/cifra/sha2.h"
#include "cryptography/cifra/aes.h"
#include "cryptography/uECC/uECC.h"

#include "utils/hash_utils.h"

#include "ff.h"
#include "f_util.h"
#include "sd_card.h"
#include <string.h>
#include <stdio.h>


#define PASSWORD_BLOCK_LENGTH   (16)
#define BASE_KEY_INDEX          (44)


//
// Wallet file format
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

#define WALLET_FILE_VERSION_MAJOR   (0x00)
#define WALLET_FILE_VERSION_MINOR   (0x01)

const uint16_t WALLET_FILE_VERSION  = ((WALLET_FILE_VERSION_MAJOR << 8) | WALLET_FILE_VERSION_MINOR);
static const char* const WALLET_FILE       = "wallet.dat";
static const char* const WALLET_DIRECTORY  = "PicoWallet";


// Hardware Configuration of SPI "objects"
// Note: multiple SD cards can be driven by one SPI if they use different slave
// selects.
static spi_t SDCARD_SPI = {
    .hw_inst    = spi0,                 // SPI component
    .miso_gpio  = 4,
    .mosi_gpio  = 7,
    .sck_gpio   = 6,
    .baud_rate  = 12500 * 1000          //.baud_rate = 25 * 1000 * 1000, // Actual frequency: 20833333. 
};

// Hardware Configuration of the SD Card "objects"
static sd_card_t SD_CARD = {
    .pcName = "0:",                     // Name used to mount device
    .spi = &SDCARD_SPI,                 // Pointer to the SPI driving this card
    .ss_gpio = 5,                       // The SPI slave select GPIO for this SD card
    .use_card_detect    = false,
    .card_detect_gpio   = 13,           // TODO: Card detect
    .card_detected_true = -1            // What the GPIO read returns when a card is
                                        // present. Use -1 if there is no card detect.
};

static cf_aes_context aesContext;


void pad_password(const char* passwordSrc, char* passwordDst) {
    for(int dst = 0, src = 0; dst < PASSWORD_BLOCK_LENGTH; ++dst) {
        passwordDst[dst] = passwordSrc[src];
        if(++src >= USER_PASSWORD_LENGTH) {
            src = 0;
        }
    }
}

size_t sd_get_num() {
    return 1; 
}

sd_card_t *sd_get_by_num(size_t num) {
    return (num <= sd_get_num()) ? &SD_CARD : NULL;
}

size_t spi_get_num() { 
    return 1;
}

spi_t *spi_get_by_num(size_t num) {
    return (num <= sd_get_num()) ? &SDCARD_SPI : NULL;
}

int wallet_to_bytes(const HDWallet* wallet, uint8_t* dest) {
    uint8_t* writePtr = dest;
    uint8_t padBytes;

    // Write version code
    memcpy(dest, &WALLET_FILE_VERSION, sizeof(WALLET_FILE_VERSION));
    writePtr += sizeof(WALLET_FILE_VERSION);

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

wallet_error wallet_from_bytes(HDWallet* wallet, uint8_t* src) {
    uint8_t* readPtr = src;
    uint16_t version;
    uint8_t workBuffer[32];

    // Read and validate version code
    memcpy(&version, readPtr, sizeof(version));
    readPtr += sizeof(version);
    if(version != WALLET_FILE_VERSION) {
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

    // Read mnemonic
    for(int i = 0; i < MNEMONIC_LENGTH; ++i) {
        strncpy(wallet->mnemonicSentence[i], readPtr, MAX_MNEMONIC_WORD_LENGTH + 1);
        readPtr += (MAX_MNEMONIC_WORD_LENGTH + 1);
    }

    return NO_ERROR;
}


FRESULT open_wallet_file(FIL* file, int write) {
    FRESULT fr;
    sd_card_t* sd = sd_get_by_num(0);
    
    fr = f_mount(&sd->fatfs, sd->pcName, 1);
    if(FR_OK != fr) {
        printf("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
        return fr;
    }

    fr = f_chdir(WALLET_DIRECTORY);
    if(FR_OK != fr) {
        if((FR_NO_PATH == fr) || (FR_NO_FILE == fr)) {
            fr = f_mkdir(WALLET_DIRECTORY);
            if(FR_OK != fr) {
                printf("f_mkdir error for %s: %s (%d)\n", WALLET_DIRECTORY, (fr), fr);
                f_unmount(sd->pcName);
                return fr;
            }
        } else {
            printf("f_chdir error for %s: %s (%d)\n", WALLET_DIRECTORY, (fr), fr);
            f_unmount(sd->pcName);
            return fr;
        }

        fr = f_chdir(WALLET_DIRECTORY);
    }

    if(FR_OK != fr) {
        printf("f_chdir error for %s: %s (%d)\n", WALLET_DIRECTORY, (fr), fr);
        f_unmount(sd->pcName);
        return fr;
    }

    if(write) {
        return f_open(file, WALLET_FILE, (FA_OPEN_ALWAYS | FA_WRITE));
    } else {
        return f_open(file, WALLET_FILE, (FA_OPEN_EXISTING | FA_READ));
    }
}

FRESULT close_wallet_file(FIL* file) {
    sd_card_t* sd = sd_get_by_num(0);

    FRESULT fr = f_close(file);
    if (FR_OK != fr) {
        printf("f_close error: %s (%d)\n", FRESULT_str(fr), fr);
    }
    f_unmount(sd->pcName);
}

int init_new_wallet(HDWallet* wallet, const uint8_t* password, const uint8_t* mnemonic, int mnemonicLen) {
    generate_master_key(mnemonic, mnemonicLen, &wallet->masterKey, wallet->mnemonicSentence);
    memcpy(wallet->password, password, USER_PASSWORD_LENGTH);
    derive_child_key(&wallet->masterKey, BASE_KEY_INDEX, &wallet->baseKey44);
    return 1;
}

wallet_error save_wallet(const HDWallet* wallet, uint8_t* fileBuffer) {
    FRESULT openResult, closeResult, writeResult;
    uint8_t workBuffer[AES_BLOCKSZ];
    uint8_t paddedPassword[PASSWORD_BLOCK_LENGTH];
    FIL walletFile;
    UINT bytesWritten;
    int bytesToWrite;
    uint8_t* writePtr;


    // Open wallet file for writing
    openResult = open_wallet_file(&walletFile, 1);
    if (FR_OK != openResult) {
        printf("f_open(%s) error: %s (%d)\n", WALLET_FILE, FRESULT_str(openResult), openResult);
        return WALLET_ERROR(WF_FAILED_TO_OPEN, openResult);
    }

    // Serialize wallet to raw bytes
    bytesToWrite = wallet_to_bytes(wallet, fileBuffer);
    writePtr = fileBuffer;

    // Encrypt and write raw bytes in chunks
    pad_password(wallet->password, paddedPassword);
    cf_aes_init(&aesContext, paddedPassword, PASSWORD_BLOCK_LENGTH);

    while(bytesToWrite) {
        // Encrypt
        cf_aes_encrypt(&aesContext, writePtr, workBuffer);
        
        // Store
        writeResult = f_write(&walletFile, workBuffer, AES_BLOCKSZ, &bytesWritten);

        // Check
        if((FR_OK != writeResult) || (bytesWritten != AES_BLOCKSZ)) {
            close_wallet_file(&walletFile);
            return WALLET_ERROR(WF_FAILED_TO_WRITE_KEY_DATA, writeResult);
        }

        // Loop
        writePtr += AES_BLOCKSZ;
        bytesToWrite -= AES_BLOCKSZ;
    }

    cf_aes_finish(&aesContext);     // Done encrypting

    closeResult = close_wallet_file(&walletFile);
    if (FR_OK != closeResult) {
        printf("f_close error: %s (%d)\n", FRESULT_str(closeResult), closeResult);
    }

    return NO_ERROR;
}

wallet_error load_wallet_bytes(uint8_t* walletBytes) {
    FRESULT openResult, closeResult, readResult;
    uint8_t* writePtr = walletBytes;
    UINT bytesRead;
    FIL walletFile;
    wallet_error returnValue = NO_ERROR;

    openResult = open_wallet_file(&walletFile, 0);

    if(FR_OK != openResult) {
        if((FR_NO_PATH == openResult) || (FR_NO_FILE == openResult)) {
            return WALLET_ERROR(WF_FILE_NOT_FOUND, openResult);
        }

        return WALLET_ERROR(WF_FAILED_TO_OPEN, openResult);
    }

    // Read wallet data
    for (;;) {
        readResult = f_read(&walletFile, writePtr, 128, &bytesRead);
        writePtr += bytesRead;
        if(!bytesRead) {
            // Either an error or EOF occurred
            break;
        }
    }

    if(FR_OK != readResult) {
        returnValue = WALLET_ERROR(WF_FAILED_TO_READ_KEY_DATA, readResult);
    } else if((writePtr - walletBytes) != WALLET_FILE_SIZE) {
        returnValue = WALLET_ERROR(WF_WALLET_FILE_CORRUPTED, readResult);
    }

    closeResult = close_wallet_file(&walletFile);
    if (FR_OK != closeResult) {
        printf("f_close error: %s (%d)\n", FRESULT_str(closeResult), closeResult);
    }

    return returnValue;
}

wallet_error rehydrate_wallet(HDWallet* wallet, uint8_t* walletBytes) {
    wallet_error walletParseError;
    uint8_t workBuffer[64];
    uint8_t paddedPassword[PASSWORD_BLOCK_LENGTH];
    const uECC_Curve curve = uECC_secp256k1();
    uint16_t bytesToProcess = WALLET_FILE_SIZE;
    uint8_t* readPtr = walletBytes;


    // Decrypt the wallet bytes
    pad_password(wallet->password, paddedPassword);
    cf_aes_init(&aesContext, paddedPassword, PASSWORD_BLOCK_LENGTH);
    while(bytesToProcess) {
        cf_aes_decrypt(&aesContext, readPtr, workBuffer);
        memcpy(readPtr, workBuffer, AES_BLOCKSZ);
        readPtr += AES_BLOCKSZ;
        bytesToProcess -= AES_BLOCKSZ;
    }
    cf_aes_finish(&aesContext);

    // Convert the raw bytes into base wallet elements
    walletParseError = wallet_from_bytes(wallet, walletBytes);
    if(NO_ERROR != walletParseError) {
        return walletParseError;
    }

    // At this point we should have the basic wallet elements available to
    // fill out remaining key data
    wallet->masterKey.depth = 0;
    wallet->masterKey.index = 0;
    memset(wallet->masterKey.parentFingerprint, 0, FINGERPRINT_LENGTH);

    // Get and compress the public key
    int success = uECC_compute_public_key(wallet->masterKey.privateKey, workBuffer, curve);
    if(success) {
        wallet->masterKey.publicKey[0] = (workBuffer[63] & 1) ? 0x03 : 0x02;
        memcpy(&(wallet->masterKey.publicKey[1]), workBuffer, (PUBLIC_KEY_LENGTH - 1));
    } else {
        return WALLET_ERROR(WF_WALLET_FILE_CORRUPTED, 0);
    }

    // Get fingerprint
    hash_160(wallet->masterKey.publicKey, PUBLIC_KEY_LENGTH, workBuffer);
    memcpy(wallet->masterKey.fingerprint, workBuffer, FINGERPRINT_LENGTH);

    // Derive base key
    derive_child_key(&wallet->masterKey, BASE_KEY_INDEX, &wallet->baseKey44);

    return NO_ERROR;   
}
