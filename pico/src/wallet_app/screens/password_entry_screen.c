#include "wallet_defs.h"
#include "wallet_app/wallet_app.h"
#include "wallet_screen.h"
#include "waveshare_lcd/lib/GUI/GUI_Paint.h"
#include "waveshare_lcd/lib/LCD/LCD_1in44.h"
#include <string.h>


#define PASSWORD_CHAR_MIN       (0x20)
#define PASSWORD_CHAR_MAX       (0x7E)
#define TITLE_STRING            ("Enter password:")



void password_entry_screen_key_pressed(DisplayKey key);
void password_entry_screen_key_released(DisplayKey key);
void password_entry_screen_key_held(DisplayKey key);
void draw_password_entry_screen();
void password_entry_screen_enter(uint8_t* screenDataBuffer);
void password_entry_screen_exit();
void password_entry_screen_update();


typedef struct {
    uint8_t password[USER_PASSWORD_LENGTH];
    uint8_t selectedPasswordDigit;
} PasswordEntryData;


static PasswordEntryData* passwordEntryScreenData;
static sFONT* drawFont = &Font10;

static const KeyButtonType BUTTON_BAR_TYPES[] = {
    UP_KEY,
    DOWN_KEY,
    RIGHT_KEY,
    OK_KEY
};


WalletScreen gPasswordEntryScreen = {
    .screenID = PASSWORD_ENTRY_SCREEN,
    .keyPressFunction = password_entry_screen_key_pressed,
    .keyReleaseFunction = password_entry_screen_key_released,
    .keyHoldFunction = password_entry_screen_key_held,
    .screenEnterFunction = password_entry_screen_enter,
    .screenUpdateFunction = password_entry_screen_update,
    .drawFunction = draw_password_entry_screen,
    .screenExitFunction = password_entry_screen_exit,
    .screenData = NULL
};


void update_selected_char(DisplayKey key) {
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
            gPasswordEntryScreen.userInteractionCompleted = true;
            break;
    }
}

void password_entry_screen_key_pressed(DisplayKey key) {
}

void password_entry_screen_key_held(DisplayKey key) {
    update_selected_char(key);
}

void password_entry_screen_key_released(DisplayKey key) {
    update_selected_char(key);
}

void draw_password_entry_screen() {
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

void password_entry_screen_enter(uint8_t* screenDataBuffer) {
    gPasswordEntryScreen.userInteractionCompleted = false;
    passwordEntryScreenData = (PasswordEntryData*) screenDataBuffer;
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

void password_entry_screen_exit(uint8_t* outputData) {
    memcpy(outputData, passwordEntryScreenData->password, USER_PASSWORD_LENGTH);
}

void password_entry_screen_update() {}
