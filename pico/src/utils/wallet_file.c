#include "wallet_file.h"
#include "wallet_app/hd_wallet.h"
#include "ff.h"
#include "f_util.h"
#include "sd_card.h"

#include <stdio.h>


static const char* const WALLET_FILE        = "wallet9.dat";
static const char* const MNEMONICS_FILE     = "mnemonic.txt";
static const char* const WALLET_DIRECTORY   = "PicoWallet";


#define MISO_PIN                (4)
#define SD_SS_PIN               (5)              
#define SCK_PIN                 (6)
#define MOSI_PIN                (7)
#define SPI_INST                (spi0)

#define WRITE_BLOCK_SIZE        (128)

#define IS_MNEMONIC_CHAR(x)     ((x >= 'a') && (x <= 'z'))

// Hardware Configuration of SPI "objects"
// Note: multiple SD cards can be driven by one SPI if they use different slave
// selects.
static spi_t SDCARD_SPI = {
    .hw_inst    = SPI_INST,                 // SPI component
    .miso_gpio  = MISO_PIN,
    .mosi_gpio  = MOSI_PIN,
    .sck_gpio   = SCK_PIN,
    .baud_rate  = 12500 * 1000          //.baud_rate = 25 * 1000 * 1000, // Actual frequency: 20833333. 
};

// Hardware Configuration of the SD Card "objects"
static sd_card_t SD_CARD = {
    .pcName = "0:",                     // Name used to mount device
    .spi = &SDCARD_SPI,                 // Pointer to the SPI driving this card
    .ss_gpio = SD_SS_PIN,               // The SPI slave select GPIO for this SD card
    .use_card_detect    = false,
    .card_detect_gpio   = 13,           // TODO: Card detect
    .card_detected_true = -1            // What the GPIO read returns when a card is
                                        // present. Use -1 if there is no card detect.
};

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

FRESULT open_wallet_file(FIL* file, int write, const char* filename) {
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
        return f_open(file, filename, (FA_OPEN_ALWAYS | FA_WRITE));
    } else {
        return f_open(file, filename, (FA_OPEN_EXISTING | FA_READ));
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

wallet_error load_wallet_data_from_disk(uint8_t* data) {
    FRESULT openResult, closeResult, readResult;
    uint8_t* dataPtr = data;
    UINT bytesRead;
    FIL walletFile;
    wallet_error returnValue = NO_ERROR;


    openResult = open_wallet_file(&walletFile, 0, WALLET_FILE);
    if(FR_OK != openResult) {
        if((FR_NO_PATH == openResult) || (FR_NO_FILE == openResult)) {
            return WALLET_ERROR(WF_FILE_NOT_FOUND, openResult);
        }

        return WALLET_ERROR(WF_FAILED_TO_OPEN, openResult);
    }

    // Read wallet data
    for (;;) {
        readResult = f_read(&walletFile, dataPtr, WRITE_BLOCK_SIZE, &bytesRead);
        dataPtr += bytesRead;
        if(!bytesRead) {
            // Either an error or EOF occurred
            break;
        }
    }

    if(FR_OK != readResult) {
        returnValue = WALLET_ERROR(WF_FAILED_TO_READ_KEY_DATA, readResult);
    } else if((dataPtr - data) != SERIALIZED_WALLET_SIZE) {
        returnValue = WALLET_ERROR(WF_WALLET_FILE_CORRUPTED, readResult);
    }

    closeResult = close_wallet_file(&walletFile);
    if (FR_OK != closeResult) {
        printf("f_close error: %s (%d)\n", FRESULT_str(closeResult), closeResult);
    }

    return returnValue;
}

wallet_error save_wallet_data_to_disk(const uint8_t* data) {
    FRESULT openResult, closeResult, writeResult;
    FIL walletFile;
    UINT bytesWritten;
    int bytesRemaining = SERIALIZED_WALLET_SIZE;
    const uint8_t* dataPtr = data;


    // Open wallet file for writing
    openResult = open_wallet_file(&walletFile, 1, WALLET_FILE);
    if (FR_OK != openResult) {
        printf("f_open(%s) error: %s (%d)\n", WALLET_FILE, FRESULT_str(openResult), openResult);
        return WALLET_ERROR(WF_FAILED_TO_OPEN, openResult);
    }

    while(bytesRemaining) {
        int bytesToWrite = (bytesRemaining > WRITE_BLOCK_SIZE) ? WRITE_BLOCK_SIZE : bytesRemaining;
        writeResult = f_write(&walletFile, dataPtr, bytesToWrite, &bytesWritten);

        // Check
        if((FR_OK != writeResult) || (bytesWritten != bytesToWrite)) {
            close_wallet_file(&walletFile);
            return WALLET_ERROR(WF_FAILED_TO_WRITE_KEY_DATA, writeResult);
        }

        // Loop
        dataPtr += bytesWritten;
        bytesRemaining -= bytesWritten;
    }

    closeResult = close_wallet_file(&walletFile);
    if (FR_OK != closeResult) {
        printf("f_close error: %s (%d)\n", FRESULT_str(closeResult), closeResult);
    }

    return NO_ERROR;
}

wallet_error read_mnemonics_from_disk(char mnemonics[MNEMONIC_LENGTH][MAX_MNEMONIC_WORD_LENGTH + 1]) {
    FIL mnemonicsFile;
    FRESULT openResult, closeResult;
    UINT bytesRead;
    wallet_error readResult;
    wallet_error returnValue = NO_ERROR;
    int lineCount = 0, bytePos = 0;


    // Open mnemonic file
    openResult = open_wallet_file(&mnemonicsFile, 0, MNEMONICS_FILE);
    if(FR_OK != openResult) {
        return WALLET_ERROR(WF_FAILED_TO_OPEN, openResult);
    }

    // Loop until we hit our first mnemonic char. This gets rid of any leading spaces or newlines or
    // whatever
    char c;
    do {
        readResult = f_read(&mnemonicsFile, &c, 1, &bytesRead);
        if((FR_OK != readResult) || !bytesRead) {
            return WALLET_ERROR(WF_FATFS_ERROR, readResult);
        }
    } while(!IS_MNEMONIC_CHAR(c));

    // Store initial mnemonic character
    mnemonics[lineCount][bytePos++] = c;

    // Read remainder of file char by char
    int terminate = 0;
    do {
        // Loop until we get something mnemonic-y
        int gotSeparator = 0;
        do {
            readResult = f_read(&mnemonicsFile, &c, 1, &bytesRead);
            if(FR_OK != readResult) {
                // Error, close file and terminate with error
                terminate = 1;
            } else if(!bytesRead) {
                // EOF, close file and terminate
                terminate = 1;
            } else if(!IS_MNEMONIC_CHAR(c)) {
                // If we got a non-mnemonic char then that counts as a word separator.
                gotSeparator = 1;
            }
        } while(!IS_MNEMONIC_CHAR(c) && !terminate);

        // Check to see if we have reached a termination condition
        if(terminate) {
            if(FR_OK != readResult) {
                return WALLET_ERROR(WF_FATFS_ERROR, readResult);
            }
            break;
        }
        
        // If we encountered separator characters, move on to the next mnemonic slot
        if(gotSeparator) {
            mnemonics[lineCount][bytePos] = 0;
            ++lineCount;
            bytePos = 0;
        }

        // Store our mnemonic char
        mnemonics[lineCount][bytePos++] = c;
    } while((lineCount < MNEMONIC_LENGTH) && (FR_OK == readResult));

    // We have finished reading the final word. Need to add in the final null-terminator
    mnemonics[lineCount][bytePos] = 0;

    // Sanity check
    if(lineCount != (MNEMONIC_LENGTH - 1)) {
        return WALLET_ERROR(WF_BAD_MNEMONIC_FILE_DATA, 0);
    }

    return NO_ERROR;
}
