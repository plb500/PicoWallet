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
    uint8_t password[USER_PASSWORD_LENGTH];             // Encryption password/key
    char mnemonicSentence[MNEMONIC_LENGTH][MAX_MNEMONIC_WORD_LENGTH + 1];
} HDWallet;


/**
 * Create a new wallet with a brand new master key
 * 
 * password         in      Pointer to 8-character password that may be present when configuring the wallet. Optional
 * mnemonic         in      The string that may be used when generating the seed phrase. Optional.
 * mnemonicLen      in      The number of characters in the "mnemonic" parameter
 * wallet           out     The wallet to be configured
 */
int init_new_wallet(HDWallet* wallet, const uint8_t* password, const uint8_t* mnemonic, int mnemonicLen);

/**
 * Decrypt the proided wallet bytes (i.e. bytes loaded from disk) into a usable wallet instance
 *
 * data             in      Pointer to the encrypted serialied wallet bytes     
 * wallet           out     The wallet to be configured
 */
wallet_error decrypt_wallet_data(uint8_t* data, HDWallet* dest);

/**
 * Attempt to a recover a wallet from the mnemonics file on disk
 * 
 * wallet           out     The wallet to be recovered
 */
wallet_error recover_wallet(HDWallet* wallet);

/**
 * Encrypt and save the supplied wallet to disk
 * 
 * wallet           in      The wallet to be saved
 */
wallet_error save_wallet(const HDWallet* wallet);


/**
 * Store the encryption password for the supplied wallet
 * 
 * wallet           in      The wallet which shall be encrypted using the supplied password
 * password         in      Pointer to 8-character password used when encrypting the wallet contents on disk
 */
void set_wallet_password(HDWallet* wallet, const uint8_t* password);


#endif      // _HD_WALLET_H_
