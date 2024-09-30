#include "wallet_defs.h"
#include "wallet_app/wallet_app.h"
#include "wallet_screen.h"
#include "gfx/gfx_utils.h"
#include "gfx/wallet_fonts.h"

#include <string.h>


#define PASSWORD_CHAR_MIN       (0x20)
#define PASSWORD_CHAR_MAX       (0x7E)
#define TITLE_STRING            ("Enter password:")



void password_entry_screen_key_pressed(WalletScreen* screen, DisplayKey key);
void password_entry_screen_key_released(WalletScreen* screen, DisplayKey key);
void password_entry_screen_key_held(WalletScreen* screen, DisplayKey key);
void draw_password_entry_screen(WalletScreen* screen);
void password_entry_screen_enter(WalletScreen* screen);
void password_entry_screen_exit(WalletScreen* screen, void* outputData);
void password_entry_screen_update(WalletScreen* screen);


static const KeyButtonType BUTTON_BAR_TYPES[] = {
    UP_KEY,
    DOWN_KEY,
    RIGHT_KEY,
    OK_KEY
};

typedef struct {
    uint8_t password[USER_PASSWORD_LENGTH];
    uint8_t selectedPasswordDigit;
} PasswordEntryData;


void update_selected_char(WalletScreen* screen, DisplayKey key) {
    PasswordEntryData* passwordEntryScreenData = (PasswordEntryData*) screen->screenData; 
    
    switch(key) {
        case KEY_A:
            --passwordEntryScreenData->password[passwordEntryScreenData->selectedPasswordDigit];
            if(passwordEntryScreenData->password[passwordEntryScreenData->selectedPasswordDigit] < PASSWORD_CHAR_MIN) {
                passwordEntryScreenData->password[passwordEntryScreenData->selectedPasswordDigit] = PASSWORD_CHAR_MAX;
            }
            break;

        case KEY_B:
            ++passwordEntryScreenData->password[passwordEntryScreenData->selectedPasswordDigit];
            if(passwordEntryScreenData->password[passwordEntryScreenData->selectedPasswordDigit] > PASSWORD_CHAR_MAX) {
                passwordEntryScreenData->password[passwordEntryScreenData->selectedPasswordDigit] = PASSWORD_CHAR_MIN;
            }
            break;

        case KEY_C:
            ++passwordEntryScreenData->selectedPasswordDigit;
            if(passwordEntryScreenData->selectedPasswordDigit >= USER_PASSWORD_LENGTH) {
                passwordEntryScreenData->selectedPasswordDigit = 0;
            }
            break;

        case KEY_D:
            screen->exitCode = 1;
            break;
    }
}

void init_password_entry_screen(WalletScreen* screen) {
    screen->screenID = PASSWORD_ENTRY_SCREEN;
    screen->keyPressFunction = password_entry_screen_key_pressed;
    screen->keyReleaseFunction = password_entry_screen_key_released;
    screen->keyHoldFunction = password_entry_screen_key_held;
    screen->screenEnterFunction = password_entry_screen_enter;
    screen->screenUpdateFunction = password_entry_screen_update;
    screen->drawFunction = draw_password_entry_screen;
    screen->screenExitFunction = password_entry_screen_exit;
}

void password_entry_screen_key_pressed(WalletScreen* screen, DisplayKey key) {
}

void password_entry_screen_key_held(WalletScreen* screen, DisplayKey key) {
    update_selected_char(screen, key);
}

void password_entry_screen_key_released(WalletScreen* screen, DisplayKey key) {
    update_selected_char(screen, key);
}

void draw_password_entry_screen(WalletScreen* screen) {
    const WalletDisplayInfo* displayInfo = get_display_info();
    const WalletFont* passwordFont = &PW_FONT_LARGE;
    const WalletFont* messageFont = &PW_FONT_MED;

    PasswordEntryData* passwordEntryScreenData = (PasswordEntryData*) screen->screenData; 
    const uint16_t passwordWidth = (passwordFont->charWidth * USER_PASSWORD_LENGTH);
    const uint16_t messageWidth = (messageFont->charWidth * strlen(TITLE_STRING));
    int xPos = (messageWidth >= displayInfo->displayWidth) ? 0 : ((displayInfo->displayWidth - messageWidth) / 2);
    int yPos = 10;

    wallet_gfx_draw_string(xPos, yPos, TITLE_STRING, strlen(TITLE_STRING), messageFont, PW_WHITE, PW_BLACK);

    yPos += (messageFont->charHeight * 2);
    xPos = ((displayInfo->displayWidth - passwordWidth) / 2);

    for(int i = 0; i < USER_PASSWORD_LENGTH; ++i) {
        bool selectedChar = (i == passwordEntryScreenData->selectedPasswordDigit);
        uint16_t charColor = selectedChar ? PW_GREEN : PW_WHITE;
        char drawChar = selectedChar ? passwordEntryScreenData->password[i] : '*';

        wallet_gfx_draw_char(xPos, yPos, drawChar, passwordFont, charColor, PW_BLACK);
        
        if(selectedChar) {
            wallet_gfx_draw_line(
                xPos, yPos + passwordFont->charHeight + 2, 
                xPos + passwordFont->charWidth, yPos + passwordFont->charHeight + 2, 
                1, 
                PW_GREEN
            );
        }
        xPos += passwordFont->charWidth;
    }

    render_button_bar(BUTTON_BAR_TYPES);
}

void password_entry_screen_enter(WalletScreen* screen) {
    screen->exitCode = 0;
    PasswordEntryData* passwordEntryScreenData = (PasswordEntryData*) screen->screenData; 
    passwordEntryScreenData->selectedPasswordDigit = 0;

    passwordEntryScreenData->password[0] = 'p';
    passwordEntryScreenData->password[1] = 'a';
    passwordEntryScreenData->password[2] = 's';
    passwordEntryScreenData->password[3] = 's';
    passwordEntryScreenData->password[4] = 'w';
    passwordEntryScreenData->password[5] = 'o';
    passwordEntryScreenData->password[6] = 'r';
    passwordEntryScreenData->password[7] = 'd';
}

void password_entry_screen_exit(WalletScreen* screen, void* outputData) {
    PasswordEntryData* passwordEntryScreenData = (PasswordEntryData*) screen->screenData; 
    memcpy(outputData, passwordEntryScreenData->password, USER_PASSWORD_LENGTH);
}

void password_entry_screen_update(WalletScreen* screen) {}
