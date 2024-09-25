#ifndef _ICON_MESSAGE_SCREEN_H_
#define _ICON_MESSAGE_SCREEN_H_

#include "wallet_screen.h"

typedef enum {
    ERROR,
    SUCCESS,
    INFO
} IconType;

typedef struct {
    IconType iconType;
    const char *message;
    KeyButtonType buttonKeys[NUM_KEYS];
} IconMessageScreenData;

void init_icon_message_screen(WalletScreen* screen, IconMessageScreenData data);

#endif      // _ICON_MESSAGE_SCREEN_H_
