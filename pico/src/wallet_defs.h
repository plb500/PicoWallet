#ifndef _WALLET_DEFS_H_
#define _WALLET_DEFS_H_

#include "ff.h"
#include <stdint.h>

// Error values
typedef enum {
    WF_OK                               = 0,
    WF_FAIL,
    WF_FATFS_ERROR,
    WF_FILE_NOT_FOUND,
    WF_FILE_VERSION_MISMATCH,
    WF_FAILED_TO_OPEN,
    WF_FAILED_TO_READ_KEY_DATA,
    WF_FAILED_TO_WRITE_KEY_DATA,
    WF_BAD_MNEMONIC_FILE_DATA,
    WF_MNEMONIC_CHECKSUM_INVALID,
    WF_INVALID_PASSWORD,
    WF_WALLET_FILE_CORRUPTED,
    WF_FILESYSTEM_INIT_FAILED
} wallet_file_result;

typedef uint16_t wallet_error;

// Error functions
#define WALLET_ERROR(x,y)       ((uint16_t) (((x) << 8) | (y)))
#define GET_WF_RESULT(x)        ((wallet_file_result) ((x) >> 8))
#define GET_FR_RESULT(x)        ((FRESULT) ((x) & 0xFF))
#define NO_ERROR                (WALLET_ERROR(WF_OK, FR_OK))

// Global sizes
#define USER_PASSWORD_LENGTH    (8)


#endif          // _WALLET_DEFS_H_
