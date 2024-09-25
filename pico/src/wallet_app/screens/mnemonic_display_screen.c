#include "mnemonic_display_screen.h"
#include "gfx/gfx_utils.h"
#include "gfx/wallet_fonts.h"

#include <string.h>


void mnemonic_display_screen_key_released(WalletScreen* screen, DisplayKey key);
void draw_mnemonic_display_screen(WalletScreen* screen);
void mnemonic_display_screen_enter(WalletScreen* screen);

static const KeyButtonType BUTTON_BAR_TYPES[] = {
    NO_KEY,
    NO_KEY,
    NO_KEY,
    NO_KEY
};

void init_mnemonic_display_screen(WalletScreen* screen, MnemonicMessageScreenData data) {
    screen->screenID = MNEMONIC_DISPLAY_SCREEN,
    screen->keyPressFunction = NULL,
    screen->keyReleaseFunction = mnemonic_display_screen_key_released,
    screen->keyHoldFunction = NULL,
    screen->screenEnterFunction = mnemonic_display_screen_enter,
    screen->screenExitFunction = NULL,
    screen->screenUpdateFunction = NULL,
    screen->drawFunction = draw_mnemonic_display_screen;
    memcpy(screen->screenData, &data, sizeof(MnemonicMessageScreenData));
}

void mnemonic_display_screen_key_released(WalletScreen* screen, DisplayKey key) {
    screen->exitCode = 1;
}

void draw_mnemonic_display_screen(WalletScreen* screen) {
    const WalletDisplayInfo* displayInfo = get_display_info();
    MnemonicMessageScreenData* data = (MnemonicMessageScreenData*) screen->screenData;
    const WalletFont* drawFont = &PW_FONT_SMALL;
    const uint16_t maxMnemonicWidth = (drawFont->charWidth * 8);
    const uint16_t screenWidth = displayInfo->displayWidth;
    const uint16_t leftMnemonicX = 2;
    const uint16_t rightMnemonicX = (screenWidth - maxMnemonicWidth - 2);
    uint16_t xPos;
    uint16_t yPos = 2;

    for(int i = 0; i < MNEMONIC_LENGTH; ++i) {
        if(i > 0) {
            if((i + 1) % 2) {
                yPos += drawFont->charHeight;
                xPos = leftMnemonicX;
            } else {
                xPos = rightMnemonicX;
            }
        } else {
            xPos = leftMnemonicX;
        }

        wallet_gfx_draw_string(xPos, yPos, data->mnemonicSentence[i], strlen(data->mnemonicSentence[i]), drawFont, PW_WHITE, PW_BLACK);
    }
}

void mnemonic_display_screen_enter(WalletScreen* screen) {
    screen->exitCode = 0;
}
