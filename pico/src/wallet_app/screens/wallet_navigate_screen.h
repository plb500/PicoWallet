#ifndef _WALLET_NAVIGATE_SCREEN_H_
#define _WALLET_NAVIGATE_SCREEN_H_

#include "wallet_screen.h"
#include "utils/key_utils.h"


typedef enum {
    DISPLAY_PRIVATE_KEY     = 1,
    DISPLAY_PUBLIC_KEY      = 2
} NavigateScreenExitCode;

typedef enum {
    COIN                = 0,
    ACCOUNT             = 1,
    CHANGE              = 2,
    ADDRESS_INDEX       = 3,

    NUM_DERIVATION_PATHS
} WalletDerivationPaths;

typedef struct {
    uint16_t derivationPathValues[NUM_DERIVATION_PATHS];
    ExtendedKey selectedKey;
} NavigateScreenReturnData;


void init_wallet_navigate_screen(WalletScreen* screen, ExtendedKey* baseKey, uint16_t derivationPathValues[NUM_DERIVATION_PATHS]);


#endif      // _WALLET_NAVIGATE_SCREEN_H_
