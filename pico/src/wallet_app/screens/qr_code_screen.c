#include "wallet_screen.h"
#include "gfx/gfx_utils.h"
#include "utils/key_utils.h"
#include <string.h>


static uint8_t qrBuffer[QR_CODE_BYTES];


void set_qr_code(uint8_t* qrCodeBytes) {
    memcpy(qrBuffer, qrCodeBytes, QR_CODE_BYTES);
}

void qr_screen_press(DisplayKey key) {
    // No action
}

void qr_screen_release(DisplayKey key) {
    // No action
}

void qr_screen_draw() {
    const WalletDisplayInfo* displayInfo = get_display_info();
    uint16_t currentBit = 0;

    int qrPixelWidth = (displayInfo->displayWidth / QR_CODE_SIZE);
    int qrPixelHeight = (displayInfo->displayHeight / QR_CODE_SIZE);

    int hPad = (displayInfo->displayWidth - (qrPixelWidth * QR_CODE_SIZE)) / 2;
    int vPad = (displayInfo->displayWidth - (qrPixelHeight * QR_CODE_SIZE)) / 2;
    
    int xPos = hPad;
    int yPos = vPad;

    for (uint8_t y = 0; y < QR_CODE_SIZE; y++, yPos += qrPixelHeight) {
        for (uint8_t x = 0; x < QR_CODE_SIZE; x++, xPos += qrPixelWidth) {
            uint8_t cByte = currentBit / 8;
            uint8_t cBit = currentBit % 8;

            uint8_t set = (qrBuffer[cByte] & (1 << (7 - cBit)));
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
