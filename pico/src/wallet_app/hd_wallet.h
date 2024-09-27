#ifndef _HD_WALLET_H_
#define _HD_WALLET_H_

#include "wallet_defs.h"
#include "utils/key_utils.h"

#include <stdint.h>


#define SERIALIZED_WALLET_SIZE            (320)

extern const uint16_t WALLET_VERSION;

typedef struct {
    ExtendedKey masterKey;                              // The root, master key
    ExtendedKey baseKey44;                              // BIP44 Base key from which all keys derive ("purpose" = 44)
    uint8_t password[USER_PASSWORD_LENGTH];
    char mnemonicSentence[MNEMONIC_LENGTH][MAX_MNEMONIC_WORD_LENGTH + 1];
} HDWallet;


/**
 * Create a new wallet with a brand new master key
 * 
 * mnemonic         in      The string that may be used when generating the seed phrase. Optional.
 * mnemonicLen      in      The number of characters in the "mnemonic" parameter
 */
int init_new_wallet(HDWallet* wallet, const uint8_t* password, const uint8_t* mnemonic, int mnemonicLen);

wallet_error decrypt_wallet_data(uint8_t* data, HDWallet* dest);

wallet_error recover_wallet(HDWallet* wallet);

/**
 * Encrypt and save the supplied wallet to disk
 */
wallet_error save_wallet(const HDWallet* wallet);









/**
 * Load the raw, encrypted file bytes from wallet.dat
 */
wallet_error load_wallet_bytes(uint8_t* walletBytes);

/**
 * Convert encrypted wallet.dat file data into a working HDWallet
 */
wallet_error rehydrate_wallet(HDWallet* wallet, uint8_t* walletBytes);



#endif      // _HD_WALLET_H_
