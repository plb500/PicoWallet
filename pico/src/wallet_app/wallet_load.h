#ifndef _WALLET_LOAD_H_
#define _WALLET_LOAD_H_

#include "wallet_app/hd_wallet.h"
#include "wallet_app/screens/wallet_screen.h"


typedef enum {
    PW_INITIAL_APP_STATE,
    PW_LOAD_WALLET_FILE_STATE,
    PW_GET_PASSWORD_FOR_LOAD_STATE,
    PW_DISPLAY_PASSWORD_ERROR_FOR_LOAD_STATE,
    PW_GET_PASSWORD_FOR_CREATE_STATE,
    PW_CREATE_WALLET_FILE_STATE,
    PW_TERMINAL_ERROR_STATE,
    PW_DISPLAY_CREATED_MNEMONIC_STATE,
    PW_WALLET_LOADED_READY_STATE,
    PW_WALLET_CREATED_READY_STATE
} WalletLoadState;


typedef struct {
    WalletLoadState currentState;
    WalletScreen* currentScreen;
    uint8_t walletFileBuffer[128];
    uint8_t screenDataBuffer[128];
    HDWallet wallet;
    wallet_error walletLoadError;
} WalletLoadStateController;

void init_wallet_load_state_controller(WalletLoadStateController* controller);
void update_wallet_load_state_controller(WalletLoadStateController* controller);
bool is_wallet_load_complete(WalletLoadStateController* controller);

#endif      // _WALLET_LOAD_H_
