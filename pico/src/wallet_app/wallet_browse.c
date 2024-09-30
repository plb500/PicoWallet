#include "wallet_browse.h"
#include "screens/wallet_navigate_screen.h"
#include "screens/qr_code_screen.h"

#include <stdio.h>


static NavigateScreenReturnData cachedNavScreenData = {
    .derivationPathValues = {0, 0, 0, 0}
};


void do_browse_wallet_state_update(WalletBrowserStateController* controller);
void do_qr_code_screen_state_update(WalletBrowserStateController* controller);


void init_wallet_browser_state_controller(WalletBrowserStateController* controller) {
    controller->currentState = PW_NAVIGATING_WALLET_STATE;
    
    init_wallet_navigate_screen(controller->currentScreen, &controller->wallet->baseKey44, cachedNavScreenData.derivationPathValues);
    controller->currentScreen->screenEnterFunction(controller->currentScreen);
}

void update_wallet_browser_state_controller(WalletBrowserStateController* controller) {
    if(controller->currentScreen && controller->currentScreen->screenUpdateFunction) {
        controller->currentScreen->screenUpdateFunction(controller->currentScreen);
    }

    switch(controller->currentState) {
        case PW_NAVIGATING_WALLET_STATE:
            do_browse_wallet_state_update(controller);
            break;
        case PW_DISPLAYING_PRIVATE_KEY_QR:
            do_qr_code_screen_state_update(controller);
            break;
        case PW_DISPLAYING_PUBLIC_KEY_QR:
            do_qr_code_screen_state_update(controller);
            break;
    }
}

void do_browse_wallet_state_update(WalletBrowserStateController* controller) {
    assert(controller->currentScreen->screenID == WALLET_BROWSE_SCREEN);

    if(controller->currentScreen->exitCode) {
        controller->currentScreen->screenExitFunction(controller->currentScreen, &cachedNavScreenData);
        bool privateKey = controller->currentScreen->exitCode == DISPLAY_PRIVATE_KEY;

        init_qr_code_screen(controller->currentScreen, &cachedNavScreenData.selectedKey, privateKey);
        controller->currentScreen->screenEnterFunction(controller->currentScreen);
        controller->currentState = privateKey ? PW_DISPLAYING_PRIVATE_KEY_QR : PW_DISPLAYING_PUBLIC_KEY_QR;
    }
}

void do_qr_code_screen_state_update(WalletBrowserStateController* controller) {
    assert(controller->currentScreen->screenID == QR_CODE_SCREEN);

    if(controller->currentScreen->exitCode) {
        controller->currentState = PW_NAVIGATING_WALLET_STATE;
        
        init_wallet_navigate_screen(controller->currentScreen, &controller->wallet->baseKey44, cachedNavScreenData.derivationPathValues);
        controller->currentScreen->screenEnterFunction(controller->currentScreen);
    }
}
