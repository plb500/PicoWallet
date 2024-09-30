#ifndef _WALLET_BROWSE_H_
#define _WALLET_BROWSE_H_

#include "screens/wallet_screen.h"
#include "hd_wallet.h"


typedef enum {
    PW_NAVIGATING_WALLET_STATE,
    PW_DISPLAYING_PRIVATE_KEY_QR,
    PW_DISPLAYING_PUBLIC_KEY_QR
} WalletBrowserState;


typedef struct {
    WalletBrowserState currentState;
    WalletScreen* currentScreen;
    HDWallet* wallet;
} WalletBrowserStateController;


void init_wallet_browser_state_controller(WalletBrowserStateController* controller);
void update_wallet_browser_state_controller(WalletBrowserStateController* controller);


#endif      // _WALLET_BROWSE_H_
