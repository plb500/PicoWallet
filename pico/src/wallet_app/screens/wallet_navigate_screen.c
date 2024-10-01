#include "wallet_navigate_screen.h"
#include "gfx/wallet_fonts.h"

#include <stdio.h>
#include <string.h>


typedef struct {
    ExtendedKey* baseKey;
    uint16_t derivationPathIndices[NUM_DERIVATION_PATHS];
    uint8_t selectedDerivationPath;
} NavigateScreenData;


static const char* MAIN_SCREEN_TITLE = "Browse Keys";

static const char* DERIVATION_PATH_TITLES[] = {
    "Coin",
    "Account",
    "Change",
    "Index"
};

static const char* DERIVATION_PATH_VALUE_FORMATS[] = {
    "%04d",
    "%04d",
    "%1d",
    "%04d"
};

static const int16_t DERIVATION_PATH_MAX[] = {
    -1,
    -1,
    1,
    -1
};

static const bool DERIVATION_PATH_HARDENED[] = {
    true,
    true,
    false,
    false
};

static const KeyButtonType BUTTON_BAR_TYPES[] = {
    DOWN_KEY,
    MINUS_KEY,
    PLUS_KEY,
    QR_CODE_KEY
};


static ExtendedKey DERIVATION_PATH_KEYS[NUM_DERIVATION_PATHS];


void wallet_navigate_screen_key_released(WalletScreen* screen, DisplayKey key);
void wallet_navigate_screen_key_held(WalletScreen* screen, DisplayKey key);
void draw_wallet_navigate_screen(WalletScreen* screen);
void wallet_navigate_screen_enter(WalletScreen* screen);
void wallet_navigate_screen_exit(WalletScreen* screen, void* outputData);
void wallet_navigate_screen_update(WalletScreen* screen);


void handle_nav_button_interaction(WalletScreen* screen, DisplayKey key) {
    NavigateScreenData* navScreenData = (NavigateScreenData*) screen->screenData;

    switch(key) {
        case KEY_A:
            navScreenData->selectedDerivationPath += 1;
            if(navScreenData->selectedDerivationPath >= NUM_DERIVATION_PATHS) {
                navScreenData->selectedDerivationPath = 0;
            }
            break;
        case KEY_B:
            if(navScreenData->derivationPathIndices[navScreenData->selectedDerivationPath] > 0) {
                navScreenData->derivationPathIndices[navScreenData->selectedDerivationPath] -= 1;
            }
            break;
        case KEY_C:
            if(
                (DERIVATION_PATH_MAX[navScreenData->selectedDerivationPath] < 0) ||
                (navScreenData->derivationPathIndices[navScreenData->selectedDerivationPath] < DERIVATION_PATH_MAX[navScreenData->selectedDerivationPath])
            ) {
                navScreenData->derivationPathIndices[navScreenData->selectedDerivationPath] += 1;
            }
            break;
    }
}

void update_keys(WalletScreen* screen) {
    NavigateScreenData* navScreenData = (NavigateScreenData*) screen->screenData;

    for(int i = 0; i < NUM_DERIVATION_PATHS; ++i) {
        const ExtendedKey* parentKey = (i == 0) ? navScreenData->baseKey : &DERIVATION_PATH_KEYS[i - 1];
        bool indexHardened = DERIVATION_PATH_HARDENED[i];

        derive_child_key(
            parentKey, 
            navScreenData->derivationPathIndices[i], 
            indexHardened,
            &DERIVATION_PATH_KEYS[i]
        );
    }
}


void init_wallet_navigate_screen(WalletScreen* screen, ExtendedKey* baseKey, uint8_t selectedDerivationPath, uint16_t derivationPathValues[NUM_DERIVATION_PATHS]) {
    NavigateScreenData* navScreenData = (NavigateScreenData*) screen->screenData;

    screen->screenID = WALLET_BROWSE_SCREEN,
    screen->keyPressFunction = NULL,
    screen->keyReleaseFunction = wallet_navigate_screen_key_released,
    screen->keyHoldFunction = wallet_navigate_screen_key_held,
    screen->screenEnterFunction = wallet_navigate_screen_enter,
    screen->screenExitFunction = wallet_navigate_screen_exit,
    screen->screenUpdateFunction = NULL,
    screen->drawFunction = draw_wallet_navigate_screen;

    navScreenData->baseKey = baseKey;
    navScreenData->selectedDerivationPath = selectedDerivationPath;
    memcpy(navScreenData->derivationPathIndices, derivationPathValues, sizeof(uint16_t) * NUM_DERIVATION_PATHS);
}

void wallet_navigate_screen_key_released(WalletScreen* screen, DisplayKey key) {
    NavigateScreenData* navScreenData = (NavigateScreenData*) screen->screenData;
    if(key == KEY_D) {
        update_keys(screen);
        screen->exitCode = DISPLAY_PUBLIC_KEY;
    } else {
        handle_nav_button_interaction(screen, key);
    }
}

void wallet_navigate_screen_key_held(WalletScreen* screen, DisplayKey key) {
    NavigateScreenData* navScreenData = (NavigateScreenData*) screen->screenData;
    if(key == KEY_D) {
        update_keys(screen);
        screen->exitCode = DISPLAY_PRIVATE_KEY;
    } else {
        handle_nav_button_interaction(screen, key);
    }
}

void draw_wallet_navigate_screen(WalletScreen* screen) {
    const WalletDisplayInfo* displayInfo = get_display_info();
    NavigateScreenData* navScreenData = (NavigateScreenData*) screen->screenData;
    const WalletFont* drawFont = &PW_FONT_MED;
    char lineData[24];
    const uint16_t INIT_LINE_POS_X = 8;
    uint16_t xPos = INIT_LINE_POS_X;
    uint16_t yPos = 4;

    // Draw title
    const WalletFont* titleFont = &PW_FONT_MED;
    uint16_t titleLength = strlen(MAIN_SCREEN_TITLE);
    uint16_t titleWidth = (titleFont->charWidth * titleLength);
    uint16_t titleX = (displayInfo->displayWidth - titleWidth) / 2;
    wallet_gfx_draw_string(titleX, yPos, MAIN_SCREEN_TITLE, strlen(MAIN_SCREEN_TITLE), titleFont, PW_MAGENTA, PW_BLACK);
    yPos += (titleFont->charHeight * 2);


    for(int i = 0; i < NUM_DERIVATION_PATHS; ++i) {
        xPos = INIT_LINE_POS_X;

        char linePrefix = (i == navScreenData->selectedDerivationPath) ? '>' : ' ';
        sprintf(lineData, "%c ", linePrefix);
        wallet_gfx_draw_string(xPos, yPos, lineData, strlen(lineData), drawFont, PW_GREEN, PW_BLACK);
        xPos += (strlen(lineData) * drawFont->charWidth);

        const char* pathTitle = DERIVATION_PATH_TITLES[i];
        sprintf(lineData, "%s: ", pathTitle);
        wallet_gfx_draw_string(xPos, yPos, lineData, strlen(lineData), drawFont, PW_TEAL, PW_BLACK);
        xPos += (strlen(lineData) * drawFont->charWidth);

        sprintf(lineData, DERIVATION_PATH_VALUE_FORMATS[i],
            navScreenData->derivationPathIndices[i]
        );
        wallet_gfx_draw_string(xPos, yPos, lineData, strlen(lineData), drawFont, PW_WHITE, PW_BLACK);
        yPos += drawFont->charHeight + 1;
    }

    render_button_bar(BUTTON_BAR_TYPES);
}

void wallet_navigate_screen_enter(WalletScreen* screen) {
    NavigateScreenData* navScreenData = (NavigateScreenData*) screen->screenData;
    
    screen->exitCode = 0;
}

void wallet_navigate_screen_exit(WalletScreen* screen, void* outputData) {
    NavigateScreenData* navScreenData = (NavigateScreenData*) screen->screenData;
    NavigateScreenReturnData* returnValue = (NavigateScreenReturnData*) outputData;

    returnValue->selectedDerivationPath = navScreenData->selectedDerivationPath;
    memcpy(&returnValue->derivationPathIndices, navScreenData->derivationPathIndices, sizeof(navScreenData->derivationPathIndices));
    memcpy(&returnValue->selectedKey, &DERIVATION_PATH_KEYS[navScreenData->selectedDerivationPath], sizeof(ExtendedKey));
}

void wallet_navigate_screen_update(WalletScreen* screen) {}
