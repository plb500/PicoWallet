#include "icon_message_screen.h"
#include "waveshare_lcd/lib/GUI/GUI_Paint.h"
#include "waveshare_lcd/lib/LCD/LCD_1in44.h"
#include "pico/time.h"

#include <string.h>


void icon_message_screen_key_released(WalletScreen* screen, DisplayKey key);
void draw_icon_message_screen(WalletScreen* screen);
void icon_message_screen_enter(WalletScreen* screen);
void icon_message_screen_update(WalletScreen* screen);


extern const uint8_t ERROR_ICON[];
extern const uint8_t INFO_ICON[];
extern const uint8_t SUCCESS_ICON[];


void init_icon_message_screen(WalletScreen* screen, IconMessageScreenData data) {
    screen->screenID = ICON_MESSAGE_SCREEN,
    screen->keyPressFunction = NULL,
    screen->keyReleaseFunction = icon_message_screen_key_released,
    screen->keyHoldFunction = NULL,
    screen->screenEnterFunction = icon_message_screen_enter,
    screen->screenExitFunction = NULL,
    screen->screenUpdateFunction = NULL,
    screen->drawFunction = draw_icon_message_screen;
    memcpy(screen->screenData, &data, sizeof(IconMessageScreenData));
}

void icon_message_screen_key_released(WalletScreen* screen, DisplayKey key) {
    screen->exitCode = 1;
}

void draw_icon_message_screen(WalletScreen* screen) {
    const uint8_t iconDimensions = 48;
    const uint8_t iconX = (LCD_1IN44_WIDTH - iconDimensions) / 2;
    IconMessageScreenData* data = (IconMessageScreenData*) screen->screenData;

    int messageWidth = (Font12.Width * strlen(data->message));
    int msgX = (messageWidth >= LCD_1IN44_WIDTH) ? 0 : ((LCD_1IN44_WIDTH - messageWidth) / 2);

    int yPos = 10;
    Paint_DrawString_EN(msgX, yPos, data->message, &Font12, WHITE, BLACK);
    yPos += Font12.Height;

    yPos += (((LCD_1IN44_HEIGHT - yPos) - iconDimensions) / 2);

    const uint8_t* iconImage = 0;
    switch(data->iconType) {
        case ERROR:
            iconImage = ERROR_ICON;
            break;
        case INFO:
            iconImage = INFO_ICON;
            break;
        case SUCCESS:
            iconImage = SUCCESS_ICON;
            break;
    }

    if(iconImage) {
        Paint_DrawImage(iconImage, iconX, yPos, iconDimensions, iconDimensions);
    }
}

void icon_message_screen_enter(WalletScreen* screen) {
    screen->exitCode = 0;
}
