#include "wallet_screen.h"
#include "waveshare_lcd/lib/GUI/GUI_Paint.h"
#include "waveshare_lcd/lib/LCD/LCD_1in44.h"

#include <string.h>


void info_message_screen_key_released(DisplayKey key);
void draw_info_message_screen();
void info_message_screen_enter(uint8_t* screenDataBuffer);
void info_message_screen_exit(uint8_t* outputData);


static uint8_t* infoMessage;

static const KeyButtonType BUTTON_BAR_TYPES[] = {
    NO_KEY,
    NO_KEY,
    NO_KEY,
    OK_KEY
};

WalletScreen gInfoMessageScreen = {
    .screenID = INFO_MESSAGE_SCREEN,
    .keyPressFunction = NULL,
    .keyReleaseFunction = info_message_screen_key_released,
    .keyHoldFunction = NULL,
    .screenEnterFunction = info_message_screen_enter,
    .screenExitFunction = info_message_screen_exit,
    .screenUpdateFunction = NULL,
    .drawFunction = draw_info_message_screen,
    .screenData = NULL
};


void info_message_screen_key_released(DisplayKey key) {
    gInfoMessageScreen.userInteractionCompleted = true;
}

void draw_info_message_screen() {
    int messageWidth = (Font12.Width * strlen(infoMessage));
    int msgX = (messageWidth >= LCD_1IN44_WIDTH) ? 0 : ((LCD_1IN44_WIDTH - messageWidth) / 2);
    int msgY = 10;

    Paint_DrawString_EN(msgX, msgY, infoMessage, &Font12, WHITE, BLACK);
    render_button_bar(BUTTON_BAR_TYPES);
}

void info_message_screen_enter(uint8_t* screenDataBuffer) {
    gInfoMessageScreen.userInteractionCompleted = false;
    infoMessage = screenDataBuffer;
}

void info_message_screen_exit(uint8_t* outputData) {
    gInfoMessageScreen.userInteractionCompleted = false;
    infoMessage = NULL;
}
