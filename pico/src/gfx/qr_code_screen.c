#include "gfx_utils.h"
#include "utils/key_utils.h"
#include "waveshare_lcd/lib/GUI/GUI_Paint.h"
#include "waveshare_lcd/lib/LCD/LCD_1in44.h"
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
    uint16_t currentBit = 0;

    int qrPixelWidth = (LCD_1IN44.WIDTH / QR_CODE_SIZE);
    int qrPixelHeight = (LCD_1IN44.HEIGHT / QR_CODE_SIZE);

    int hPad = (LCD_1IN44.WIDTH - (qrPixelWidth * QR_CODE_SIZE)) / 2;
    int vPad = (LCD_1IN44.HEIGHT - (qrPixelHeight * QR_CODE_SIZE)) / 2;
    
    int xPos = hPad;
    int yPos = vPad;

    for (uint8_t y = 0; y < QR_CODE_SIZE; y++, yPos += qrPixelHeight) {
        for (uint8_t x = 0; x < QR_CODE_SIZE; x++, xPos += qrPixelWidth) {
            uint8_t cByte = currentBit / 8;
            uint8_t cBit = currentBit % 8;

            uint8_t set = (qrBuffer[cByte] & (1 << (7 - cBit)));
            UWORD color = (set == 0) ? WHITE : BLACK;

            Paint_DrawRectangle(xPos, yPos, (xPos + qrPixelWidth), (yPos + qrPixelHeight), color, DOT_PIXEL_1X1, DRAW_FILL_FULL);

            ++currentBit;
        }
        xPos = hPad;
    }
}

WalletScreen qrScreen = {
    .screenID = 0,
    .drawFunction = qr_screen_draw,
    .keyPressFunction = qr_screen_press,
    .keyReleaseFunction = qr_screen_release
};
