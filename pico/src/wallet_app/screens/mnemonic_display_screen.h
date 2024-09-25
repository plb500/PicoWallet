#ifndef _MNEMONIC_DISPLAY_SCREEN_H_
#define _MNEMONIC_DISPLAY_SCREEN_H_

#include "wallet_screen.h"
#include "utils/key_utils.h"


typedef struct {
    const char *mnemonicSentence[MNEMONIC_LENGTH];
} MnemonicMessageScreenData;

void init_mnemonic_display_screen(WalletScreen* screen, MnemonicMessageScreenData data);


#endif      // _MNEMONIC_DISPLAY_SCREEN_H_
