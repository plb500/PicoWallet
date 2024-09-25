#include "info_message_screen.h"
#include "gfx/gfx_utils.h"
#include "gfx/wallet_fonts.h"

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
    const WalletDisplayInfo* displayInfo = get_display_info();
    const WalletFont* drawFont = &PW_FONT_MED;
    InfoMessageScreenData* data = (InfoMessageScreenData*) screen->screenData;
    int messageLength = strlen(data->message);
    int messageWidth = (drawFont->charWidth * messageLength);
    int msgX = (messageWidth >= displayInfo->displayWidth) ? 0 : ((displayInfo->displayWidth - messageWidth) / 2);
    int msgY = 10;

    wallet_gfx_draw_string(msgX, msgY, data->message, messageWidth, drawFont, PW_WHITE, PW_BLACK);

    render_button_bar(BUTTON_BAR_TYPES);
}

void info_message_screen_enter(WalletScreen* screen) {
    screen->exitCode = 0;
}
