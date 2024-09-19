#include "wallet_file.h"

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


const char* PASSWORD_SYMBOLS_LC     = "abcdefghijklmnopqrstuvwxyz";
const char* PASSWORD_SYMBOLS_UC     = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char* PASSWORD_SYMBOLS_NUM    = "0123456789";
const char* PASSWORD_SYMBOLS_SYM    = "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";


// Hardware Configuration of SPI "objects"
// Note: multiple SD cards can be driven by one SPI if they use different slave
// selects.
spi_t SDCARD_SPI = {
    .hw_inst    = spi0,                 // SPI component
    .miso_gpio  = 4,
    .mosi_gpio  = 7,
    .sck_gpio   = 6,
    .baud_rate  = 12500 * 1000          //.baud_rate = 25 * 1000 * 1000, // Actual frequency: 20833333. 
};

// Hardware Configuration of the SD Card "objects"
sd_card_t SD_CARD = {
    .pcName = "0:",                     // Name used to mount device
    .spi = &SDCARD_SPI,                 // Pointer to the SPI driving this card
    .ss_gpio = 5,                       // The SPI slave select GPIO for this SD card
    .use_card_detect    = false,
    .card_detect_gpio   = 13,           // TODO: Card detect
    .card_detected_true = -1            // What the GPIO read returns when a card is
                                        // present. Use -1 if there is no card detect.
};

const char* const WALLET_FILE       = "wallet.dat";
const char* const WALLET_DIRECTORY  = "PicoWallet";

cf_aes_context aesContext;

size_t sd_get_num() {
    return 1; 
}

sd_card_t *sd_get_by_num(size_t num) {
    if (num <= sd_get_num()) {
        return &SD_CARD;
    } else {
        return NULL;
    }
}

size_t spi_get_num() { 
    return 1;
}

spi_t *spi_get_by_num(size_t num) {
    if (num <= sd_get_num()) {
        return &SDCARD_SPI;
    } else {
        return NULL;
    }
}

FRESULT open_wallet_file(FIL* file, int write) {
    FRESULT fr;
    sd_card_t* sd = sd_get_by_num(0);
    
    fr = f_mount(&sd->fatfs, sd->pcName, 1);
    if(FR_OK != fr) {
        printf("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
        return fr;
    }

    fr = f_stat (WALLET_DIRECTORY, NULL);	
    if((FR_NO_PATH == fr) || (FR_NO_FILE == fr)) {
        fr = f_mkdir(WALLET_DIRECTORY);
        if(FR_OK != fr) {
            printf("f_mkdir error for %s: %s (%d)\n", WALLET_DIRECTORY, (fr), fr);
            f_unmount(sd->pcName);
            return fr;
        }
    } else if(FR_OK != fr) {
        printf("f_stat error for %s: %s (%d)\n", WALLET_DIRECTORY, (fr), fr);
        f_unmount(sd->pcName);
        return fr;
    }

    fr = f_chdir(WALLET_DIRECTORY);
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

void encrypt_data(const uint8_t* key, int keySize, const uint8_t* inBuffer, int bufferSize, uint8_t* outBuffer) {
    int bytesRemaining = bufferSize;
    const uint8_t* in = inBuffer;
    uint8_t* out = outBuffer;

    cf_aes_init(&aesContext, key, keySize);

    while(bytesRemaining) {
        cf_aes_encrypt(&aesContext, in, out);
        in += AES_BLOCKSZ;
        out += AES_BLOCKSZ;
        bytesRemaining -= AES_BLOCKSZ;
    }

    cf_aes_finish(&aesContext);
}

void decrypt_data(const uint8_t* key, int keySize, const uint8_t* inBuffer, int bufferSize, uint8_t* outBuffer) {
    int bytesRemaining = bufferSize;
    const uint8_t* in = inBuffer;
    uint8_t* out = outBuffer;

    cf_aes_init(&aesContext, key, keySize);

    while(bytesRemaining) {
        cf_aes_decrypt(&aesContext, in, out);
        in += AES_BLOCKSZ;
        out += AES_BLOCKSZ;
        bytesRemaining -= AES_BLOCKSZ;
    }

    cf_aes_finish(&aesContext);
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
    generate_master_key(mnemonic, mnemonicLen, &wallet->masterKey);
    memcpy(wallet->password, password, PASSWORD_LENGTH);

    return 1;
}

wallet_error save_wallet(const HDWallet* wallet) {
    FRESULT openResult, closeResult, writeResult;
    FIL walletFile;
    uint8_t workBuffer[32];
    UINT bytesWritten;

    openResult = open_wallet_file(&walletFile, 1);

    if (FR_OK != openResult) {
        printf("f_open(%s) error: %s (%d)\n", WALLET_FILE, FRESULT_str(openResult), openResult);
        return WALLET_ERROR(WF_FAILED_TO_OPEN, openResult);
    }

    // Create the password hash header
    cf_pbkdf2_hmac(
        wallet->password, 8, 
        PASSCODE_PEPPER, strlen(PASSCODE_PEPPER), 
        2048, 
        workBuffer, 32,
        &cf_sha256
    );

    // Write password hash header
    writeResult = f_write(&walletFile, workBuffer, 32, &bytesWritten);
    if(bytesWritten != 32) {
        return WALLET_ERROR(WF_FAILED_TO_WRITE_KEY_DATA, writeResult);
    }

    // Encrypt and write master private key
    encrypt_data(wallet->password, 16, wallet->masterKey.privateKey, PRIVATE_KEY_LENGTH, workBuffer);
    writeResult = f_write(&walletFile, workBuffer, PRIVATE_KEY_LENGTH, &bytesWritten);
    if(bytesWritten != 32) {
        return WALLET_ERROR(WF_FAILED_TO_WRITE_KEY_DATA, writeResult);
    }

    // Encrypt and write master chain code
    encrypt_data(wallet->password, 16, wallet->masterKey.chainCode, CHAIN_CODE_LENGTH, workBuffer);
    writeResult = f_write(&walletFile, workBuffer, CHAIN_CODE_LENGTH, &bytesWritten);
    if(bytesWritten != 32) {
        return WALLET_ERROR(WF_FAILED_TO_WRITE_KEY_DATA, writeResult);
    }

    closeResult = close_wallet_file(&walletFile);
    if (FR_OK != closeResult) {
        printf("f_close error: %s (%d)\n", FRESULT_str(closeResult), closeResult);
    }
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
    }

    closeResult = close_wallet_file(&walletFile);
    if (FR_OK != closeResult) {
        printf("f_close error: %s (%d)\n", FRESULT_str(closeResult), closeResult);
    }

    return returnValue;
}

wallet_error rehydrate_wallet(HDWallet* wallet, uint8_t* walletBytes) {
    uint8_t workBuffer[64];
    int passwordCmp;
    wallet_error err;
    uint8_t* readBuffer;
    const uECC_Curve curve = uECC_secp256k1();

    // Validate hash
    cf_pbkdf2_hmac(
        wallet->password, 8, 
        PASSCODE_PEPPER, strlen(PASSCODE_PEPPER), 
        2048, 
        workBuffer, 32,
        &cf_sha256
    );

    // Compare
    passwordCmp = memcmp(workBuffer, walletBytes, 32);
    if(passwordCmp != 0) {
        return WALLET_ERROR(WF_INVALID_PASSWORD, 0);
    }

    // Decrypt master private key
    readBuffer = (walletBytes + 32);
    decrypt_data(wallet->password, 16, readBuffer, PRIVATE_KEY_LENGTH, workBuffer);
    memcpy(wallet->masterKey.privateKey, workBuffer, PRIVATE_KEY_LENGTH);

    // Decrypt master chain code
    readBuffer += PRIVATE_KEY_LENGTH;
    decrypt_data(wallet->password, 16, readBuffer, CHAIN_CODE_LENGTH, workBuffer);
    memcpy(wallet->masterKey.chainCode, workBuffer, CHAIN_CODE_LENGTH);

    // Fill out remaining key data
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

    return NO_ERROR;   
}
