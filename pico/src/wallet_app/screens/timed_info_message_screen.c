#include "wallet_screen.h"
#include "waveshare_lcd/lib/GUI/GUI_Paint.h"
#include "waveshare_lcd/lib/LCD/LCD_1in44.h"
#include "waveshare_lcd/lib/LCD/LCD_1in44.h"
#include "pico/time.h"

#include <string.h>

#define TIMEOUT_MS          (3000)

void timed_info_message_screen_key_released(DisplayKey key);
void draw_timed_info_message_screen();
void timed_info_message_screen_enter(uint8_t* screenDataBuffer);
void timed_info_message_screen_exit(uint8_t* outputData);
void timed_info_message_screen_update();


static absolute_time_t timeout;
static uint8_t* timedInfoMessage;
WalletScreen gTimedInfoMessageScreen = {
    .screenID = TIMED_INFO_MESSAGE_SCREEN,
    .keyPressFunction = NULL,
    .keyReleaseFunction = timed_info_message_screen_key_released,
    .keyHoldFunction = NULL,
    .screenEnterFunction = timed_info_message_screen_enter,
    .screenExitFunction = NULL,
    .screenUpdateFunction = timed_info_message_screen_update,
    .drawFunction = draw_timed_info_message_screen,
    .screenData = NULL
};


void timed_info_message_screen_key_released(DisplayKey key) {
    gTimedInfoMessageScreen.userInteractionCompleted = true;
}

void draw_timed_info_message_screen() {
    int messageWidth = (Font12.Width * strlen(timedInfoMessage));
    int msgX = (messageWidth >= LCD_1IN44_WIDTH) ? 0 : ((LCD_1IN44_WIDTH - messageWidth) / 2);
    int msgY = 10;

    Paint_DrawString_EN(msgX, msgY, timedInfoMessage, &Font12, WHITE, BLACK);
}

void timed_info_message_screen_enter(uint8_t* screenDataBuffer) {
    uint16_t t;
    
    memcpy(&t, screenDataBuffer, sizeof(uint16_t));
    timeout = make_timeout_time_ms(t);

    timedInfoMessage = (screenDataBuffer + sizeof(uint16_t));

    gTimedInfoMessageScreen.userInteractionCompleted = false;
}

void timed_info_message_screen_exit(uint8_t* outputData) {
    gTimedInfoMessageScreen.userInteractionCompleted = false;
    timedInfoMessage = NULL;
}

void timed_info_message_screen_update() {
    if(absolute_time_diff_us(get_absolute_time(), timeout) <= 0) {
        gTimedInfoMessageScreen.userInteractionCompleted = true;
    }
}
