#ifndef _WALLET_FILE_H_
#define _WALLET_FILE_H_

#include "utils/key_utils.h"

#include <stdint.h>


typedef struct {
    ExtendedKey masterKey;
} HDWallet;


/**
 * Create a new wallet with a brand new master key
 * 
 * mnemonic         in      The string that may be used when generating the seed phrase. Optional.
 * mnemonicLen      in      The number of characters in the "mnemonic" parameter
 */
int init_new_wallet(HDWallet* wallet, const uint8_t* mnemonic, int mnemonicLen);


#endif
