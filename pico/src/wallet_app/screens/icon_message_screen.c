#include "wallet_screen.h"
#include "waveshare_lcd/lib/GUI/GUI_Paint.h"
#include "waveshare_lcd/lib/LCD/LCD_1in44.h"
#include "pico/time.h"

#include <string.h>


#define TIMEOUT_MS          (3000)


void icon_message_screen_key_released(DisplayKey key);
void draw_icon_message_screen();
void icon_message_screen_enter(uint8_t* screenDataBuffer);
void icon_message_screen_exit(uint8_t* outputData);
void icon_message_screen_update();


extern const uint8_t ERROR_ICON[];
extern const uint8_t INFO_ICON[];
extern const uint8_t SUCCESS_ICON[];


static IconType iconType;
static uint8_t* iconMessage;
WalletScreen gIconMessageScreen = {
    .screenID = ICON_MESSAGE_SCREEN,
    .keyPressFunction = NULL,
    .keyReleaseFunction = icon_message_screen_key_released,
    .keyHoldFunction = NULL,
    .screenEnterFunction = icon_message_screen_enter,
    .screenExitFunction = NULL,
    .screenUpdateFunction = NULL,
    .drawFunction = draw_icon_message_screen,
    .screenData = NULL
};


void icon_message_screen_key_released(DisplayKey key) {
    gIconMessageScreen.userInteractionCompleted = true;
}

void draw_icon_message_screen() {
    const uint8_t iconDimensions = 48;
    const uint8_t iconX = (LCD_1IN44_WIDTH - iconDimensions) / 2;

    int messageWidth = (Font12.Width * strlen(iconMessage));
    int msgX = (messageWidth >= LCD_1IN44_WIDTH) ? 0 : ((LCD_1IN44_WIDTH - messageWidth) / 2);

    int yPos = 10;
    Paint_DrawString_EN(msgX, yPos, iconMessage, &Font12, WHITE, BLACK);
    yPos += Font12.Height;

    yPos += (((LCD_1IN44_HEIGHT - yPos) - iconDimensions) / 2);

    const uint8_t* iconImage = 0;
    if(iconType == ERROR) {
        iconImage = ERROR_ICON;
    } else if(iconType == INFO) {
        iconImage = INFO_ICON;
    } else if(iconType == SUCCESS) {
        iconImage = SUCCESS_ICON;
    }

    if(iconImage) {
        Paint_DrawImage(iconImage, iconX, yPos, iconDimensions, iconDimensions);
    }
}

void icon_message_screen_enter(uint8_t* screenDataBuffer) {
    memcpy(&iconType, screenDataBuffer, sizeof(IconType));

    iconMessage = (screenDataBuffer + sizeof(IconType));

    gIconMessageScreen.userInteractionCompleted = false;
}
