#include "timed_info_message_screen.h"
#include "waveshare_lcd/lib/GUI/GUI_Paint.h"
#include "waveshare_lcd/lib/LCD/LCD_1in44.h"
#include "waveshare_lcd/lib/LCD/LCD_1in44.h"
#include "pico/time.h"

#include <string.h>


#define TIMEOUT_MS          (3000)

void timed_info_message_screen_key_released(WalletScreen* screen, DisplayKey key);
void draw_timed_info_message_screen(WalletScreen* screen);
void timed_info_message_screen_enter(WalletScreen* screen);
void timed_info_message_screen_update(WalletScreen* screen);


void init_timed_info_message_screen(WalletScreen* screen, TimedInfoMessageScreenData data) {
    screen->screenID = TIMED_INFO_MESSAGE_SCREEN;
    screen->keyPressFunction = NULL;
    screen->keyReleaseFunction = timed_info_message_screen_key_released;
    screen->keyHoldFunction = NULL;
    screen->screenEnterFunction = timed_info_message_screen_enter;
    screen->screenExitFunction = NULL;
    screen->screenUpdateFunction = timed_info_message_screen_update;
    screen->drawFunction = draw_timed_info_message_screen;
    memcpy(screen->screenData, &data, sizeof(TimedInfoMessageScreenData));
}


void timed_info_message_screen_key_released(WalletScreen* screen, DisplayKey key) {
    screen->exitCode = 1;
}

void draw_timed_info_message_screen(WalletScreen* screen) {
    TimedInfoMessageScreenData* data = (TimedInfoMessageScreenData*) screen->screenData;

    int messageWidth = (Font12.Width * strlen(data->message));
    int msgX = (messageWidth >= LCD_1IN44_WIDTH) ? 0 : ((LCD_1IN44_WIDTH - messageWidth) / 2);
    int msgY = 10;

    Paint_DrawString_EN(msgX, msgY, data->message, &Font12, WHITE, BLACK);
}

void timed_info_message_screen_enter(WalletScreen* screen) {
    TimedInfoMessageScreenData* data = (TimedInfoMessageScreenData*) screen->screenData;
    data->timeoutExpiry = make_timeout_time_ms(data->timeoutMS);
    screen->exitCode = 0;
}

void timed_info_message_screen_update(WalletScreen* screen) {
    TimedInfoMessageScreenData* data = (TimedInfoMessageScreenData*) screen->screenData;

    if(absolute_time_diff_us(get_absolute_time(), data->timeoutExpiry) <= 0) {
        screen->exitCode = 1;
    }
}
