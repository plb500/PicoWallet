#include "wallet_screen.h"
#include "gfx/gfx_utils.h"
#include "utils/key_utils.h"
#include <string.h>


void wallet_qr_code_screen_enter(WalletScreen* screen);
void wallet_qr_code_screen_key_released(WalletScreen* screen, DisplayKey key);
void draw_qr_code_screen(WalletScreen* screen);


// Naviagting from a private key requires a key hold. This means there will be a release event waiting
// for us after we enter the screen. The latch allows us to consume the extra release event 
bool holdLatch;


void init_qr_code_screen(WalletScreen* screen, ExtendedKey* key, bool privateKey) {
    screen->screenID = QR_CODE_SCREEN,
    screen->keyPressFunction = NULL,
    screen->keyReleaseFunction = wallet_qr_code_screen_key_released,
    screen->keyHoldFunction = NULL,
    screen->screenEnterFunction = wallet_qr_code_screen_enter,
    screen->screenExitFunction = NULL,
    screen->screenUpdateFunction = NULL,
    screen->drawFunction = draw_qr_code_screen;

    holdLatch = privateKey;

    if(privateKey) {
        get_private_key_wif_qr(key, BTC_MAIN_NET, screen->screenData);
    } else {
        get_p2pkh_qr(key, screen->screenData);
    }
}

void wallet_qr_code_screen_enter(WalletScreen* screen) {
    screen->exitCode = 0;
}

void wallet_qr_code_screen_key_released(WalletScreen* screen, DisplayKey key) {
    if(holdLatch) {
        holdLatch = false;
    } else {
        screen->exitCode = 1;
    }
}

void draw_qr_code_screen(WalletScreen* screen) {
    const WalletDisplayInfo* displayInfo = get_display_info();
    uint16_t currentBit = 0;

    int qrPixelWidth = (displayInfo->displayWidth / QR_CODE_SIZE);
    int qrPixelHeight = (displayInfo->displayHeight / QR_CODE_SIZE);

    int hPad = (displayInfo->displayWidth - (qrPixelWidth * QR_CODE_SIZE)) / 2;
    int vPad = (displayInfo->displayWidth - (qrPixelHeight * QR_CODE_SIZE)) / 2;
    
    int xPos = hPad;
    int yPos = vPad;

    wallet_gfx_clear_display(PW_WHITE);
    for (uint8_t y = 0; y < QR_CODE_SIZE; y++, yPos += qrPixelHeight) {
        for (uint8_t x = 0; x < QR_CODE_SIZE; x++, xPos += qrPixelWidth) {
            uint8_t cByte = currentBit / 8;
            uint8_t cBit = currentBit % 8;

            uint8_t set = (screen->screenData[cByte] & (1 << (7 - cBit)));
            WalletPaintColor color = (set == 0) ? PW_WHITE : PW_BLACK;

            wallet_gfx_draw_rectangle(
                xPos, yPos, 
                (xPos + qrPixelWidth), (yPos + qrPixelHeight),
                1, true, color
            );

            ++currentBit;
        }
        xPos = hPad;
    }
}
