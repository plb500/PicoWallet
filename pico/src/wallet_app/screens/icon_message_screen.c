#include "icon_message_screen.h"
#include "gfx/gfx_utils.h"
#include "gfx/wallet_fonts.h"
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
    IconMessageScreenData* data = (IconMessageScreenData*) screen->screenData;

    screen->exitCode = data->buttonKeys[key];
}

void draw_icon_message_screen(WalletScreen* screen) {
    const WalletDisplayInfo* displayInfo = get_display_info();
    const WalletFont* drawFont = &PW_FONT_MED;
    const uint8_t iconDimensions = 48;
    const uint8_t iconX = (displayInfo->displayWidth - iconDimensions) / 2;

    IconMessageScreenData* data = (IconMessageScreenData*) screen->screenData;
    uint16_t iconSpaceTopMargin, iconSpaceBottomMargin;
    int hasKeys = 0;
    int messageWidth = (drawFont->charWidth * strlen(data->message));
    int msgX = (messageWidth >= displayInfo->displayWidth) ? 0 : ((displayInfo->displayWidth - messageWidth) / 2);
    int yPos = 10;

    for(int i = 0; i < NUM_KEYS; ++i) {
        if(data->buttonKeys[i] != NO_KEY) {
            hasKeys = 1;
            break;
        }
    }

    wallet_gfx_draw_string(msgX, yPos, data->message, strlen(data->message), &PW_FONT_MED, PW_WHITE, PW_BLACK);
    iconSpaceTopMargin = (yPos + drawFont->charHeight);
    iconSpaceBottomMargin = hasKeys ? (displayInfo->displayHeight - BUTTON_HEIGHT) : displayInfo->displayHeight;

    yPos = ((iconSpaceTopMargin + iconSpaceBottomMargin) / 2) - (iconDimensions / 2);

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
        wallet_gfx_draw_bitmap(iconImage, iconX, yPos, iconDimensions, iconDimensions);
    }

    if(hasKeys) {
        render_button_bar(data->buttonKeys);
    }
}

void icon_message_screen_enter(WalletScreen* screen) {
    screen->exitCode = 0;
}
