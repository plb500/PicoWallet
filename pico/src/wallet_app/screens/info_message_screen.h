#ifndef _INFO_MESSAGE_SCREEN_H_
#define _INFO_MESSAGE_SCREEN_H_

#include "wallet_screen.h"

typedef struct {
    const char *message;
} InfoMessageScreenData;

void init_info_message_screen(WalletScreen* screen, InfoMessageScreenData data);

#endif      // _ICON_MESSAGE_SCREEN_H_
