#include "wallet_defs.h"
#include "wallet_app/wallet_app.h"
#include "wallet_screen.h"
#include "waveshare_lcd/lib/GUI/GUI_Paint.h"
#include "waveshare_lcd/lib/LCD/LCD_1in44.h"
#include <string.h>


#define PASSWORD_CHAR_MIN       (0x20)
#define PASSWORD_CHAR_MAX       (0x7E)
#define TITLE_STRING            ("Enter password:")



void password_entry_screen_key_pressed(WalletScreen* screen, DisplayKey key);
void password_entry_screen_key_released(WalletScreen* screen, DisplayKey key);
void password_entry_screen_key_held(WalletScreen* screen, DisplayKey key);
void draw_password_entry_screen(WalletScreen* screen);
void password_entry_screen_enter(WalletScreen* screen);
void password_entry_screen_exit(WalletScreen* screen, uint8_t* outputData);
void password_entry_screen_update(WalletScreen* screen);


static sFONT* drawFont = &Font10;

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
    PasswordEntryData* passwordEntryScreenData = (PasswordEntryData*) screen->screenData; 
    const uint16_t charWidth = drawFont->Width;
    const uint16_t passwordWidth = (charWidth * USER_PASSWORD_LENGTH);

    int messageWidth = (Font12.Width * strlen(TITLE_STRING));
    int xPos = (messageWidth >= LCD_1IN44_WIDTH) ? 0 : ((LCD_1IN44_WIDTH - messageWidth) / 2);
    int yPos = 10;

    Paint_DrawString_EN(xPos, yPos, TITLE_STRING, &Font12, WHITE, BLACK);

    yPos += (drawFont->Height * 2);
    xPos = ((LCD_1IN44_WIDTH - passwordWidth) / 2);

    for(int i = 0; i < USER_PASSWORD_LENGTH; ++i) {
        bool selectedChar = (i == passwordEntryScreenData->selectedPasswordDigit);
        uint16_t charColor = selectedChar ? GREEN : WHITE;
        char drawChar = selectedChar ? passwordEntryScreenData->password[i] : '*';

        Paint_DrawChar(xPos, yPos, drawChar, drawFont, BLACK, charColor);
        
        if(selectedChar) {
            Paint_DrawLine(
                xPos, yPos + drawFont->Height + 2, 
                xPos + drawFont->Width, yPos + drawFont->Height + 2, 
                GREEN,
                DOT_PIXEL_1X1,
                LINE_STYLE_SOLID
            );
        }
        xPos += drawFont->Width;
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

void password_entry_screen_exit(WalletScreen* screen, uint8_t* outputData) {
    PasswordEntryData* passwordEntryScreenData = (PasswordEntryData*) screen->screenData; 
    memcpy(outputData, passwordEntryScreenData->password, USER_PASSWORD_LENGTH);
}

void password_entry_screen_update(WalletScreen* screen) {}
