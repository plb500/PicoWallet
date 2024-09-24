#include "info_message_screen.h"
#include "waveshare_lcd/lib/GUI/GUI_Paint.h"
#include "waveshare_lcd/lib/LCD/LCD_1in44.h"

#include <string.h>


void info_message_screen_key_released(WalletScreen* screen, DisplayKey key);
void draw_info_message_screen(WalletScreen* screen);
void info_message_screen_enter(WalletScreen* screen);


static const KeyButtonType BUTTON_BAR_TYPES[] = {
    NO_KEY,
    NO_KEY,
    NO_KEY,
    OK_KEY
};

void init_info_message_screen(WalletScreen* screen, InfoMessageScreenData data) {
    screen->screenID = INFO_MESSAGE_SCREEN,
    screen->keyPressFunction = NULL,
    screen->keyReleaseFunction = info_message_screen_key_released,
    screen->keyHoldFunction = NULL,
    screen->screenEnterFunction = info_message_screen_enter,
    screen->screenExitFunction = NULL,
    screen->screenUpdateFunction = NULL,
    screen->drawFunction = draw_info_message_screen;
    memcpy(screen->screenData, &data, sizeof(InfoMessageScreenData));
}

void info_message_screen_key_released(WalletScreen* screen, DisplayKey key) {
    screen->exitCode = 1;
}

void draw_info_message_screen(WalletScreen* screen) {
    InfoMessageScreenData* data = (InfoMessageScreenData*) screen->screenData;

    int messageWidth = (Font12.Width * strlen(data->message));
    int msgX = (messageWidth >= LCD_1IN44_WIDTH) ? 0 : ((LCD_1IN44_WIDTH - messageWidth) / 2);
    int msgY = 10;

    Paint_DrawString_EN(msgX, msgY, data->message, &Font12, WHITE, BLACK);

    render_button_bar(BUTTON_BAR_TYPES);
}

void info_message_screen_enter(WalletScreen* screen) {
    screen->exitCode = 0;
}
