#ifndef _WALLET_LOAD_H_
#define _WALLET_LOAD_H_

#include "wallet_app/hd_wallet.h"
#include "wallet_app/screens/wallet_screen.h"


typedef enum {
    PW_INITIAL_APP_STATE,                           // App has just started loader
    PW_LOAD_WALLET_STATE,                           // Attempting to load wallet from wallet.dat file on disk
    PW_RECOVER_WALLET_STATE,                        // Loading has failed and we are attempting to recover from mnemonic file
    PW_CREATE_WALLET_STATE,                         // Loading and recovery has failed and we are creating a new wallet
    PW_GET_PASSWORD_FOR_DECRYPT_STATE,              // Attempting to decrypt the wallet loaded from wallet.dat
    PW_GET_PASSWORD_FOR_ENCRYPT_STATE,              // Getting user password to encrypt newly created wallet
    PW_DISPLAY_PASSWORD_ERROR_FOR_LOAD_STATE,       // Decryption has failed
    PW_TERMINAL_ERROR_STATE,                        // Unrecoverable error state
    PW_DISPLAY_CREATED_MNEMONIC_STATE,              // Showing user reovery mnemonics
    PW_WALLET_READY_STATE,                          // A wallet is loaded and ready to use
} WalletLoadState;


typedef struct {
    WalletLoadState currentState;
    WalletScreen* currentScreen;
    uint8_t walletFileBuffer[SERIALIZED_WALLET_SIZE];
    uint8_t screenDataBuffer[128];
    HDWallet* wallet;
    wallet_error walletLoadError;
    bool userExitRequested;
} WalletLoadStateController;

void init_wallet_load_state_controller(WalletLoadStateController* controller);
void update_wallet_load_state_controller(WalletLoadStateController* controller);
bool is_wallet_load_complete(WalletLoadStateController* controller);

#endif      // _WALLET_LOAD_H_
