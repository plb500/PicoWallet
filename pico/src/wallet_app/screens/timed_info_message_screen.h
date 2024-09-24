#ifndef _TIMED_INFO_MESSAGE_SCREEN_H_
#define _TIMED_INFO_MESSAGE_SCREEN_H_

#include "wallet_screen.h"
#include "pico/time.h"


typedef struct {
    uint16_t timeoutMS;
    const char *message;
    absolute_time_t timeoutExpiry;
} TimedInfoMessageScreenData;


void init_timed_info_message_screen(WalletScreen* screen, TimedInfoMessageScreenData data);


#endif      // _TIMED_INFO_MESSAGE_SCREEN_H_
