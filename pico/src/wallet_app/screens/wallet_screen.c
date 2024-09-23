#include "wallet_screen.h"
#include "waveshare_lcd/lib/LCD/LCD_1in44.h"
#include "waveshare_lcd/lib/GUI/GUI_Paint.h"

#define BUTTON_WIDTH            (30)
#define BUTTON_RADIUS           (8)
#define DOT_RADIUS              (3)
#define BUTTON_HEIGHT           (15)
#define NUM_BUTTONS             (4)

void render_button_bar(const KeyButtonType types[]) {
    // Magic numbers
    const uint16_t screenWidth = LCD_1IN44_WIDTH;
    const uint16_t screenHeight = LCD_1IN44_HEIGHT;

    const uint16_t centerSpacingH = (LCD_1IN44_WIDTH / NUM_BUTTONS);
    
    uint8_t vPadding = 2;

    int yPos = screenHeight - BUTTON_RADIUS - vPadding;
    
    for(int i = 0, xPos = (centerSpacingH / 2); i < NUM_BUTTONS; ++i, xPos += centerSpacingH) {
        if(types[i] == NO_KEY) {
            continue;
        }

        Paint_DrawCircle(xPos, yPos, BUTTON_RADIUS, 0x07FD, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
        
        switch(types[i]) {
            uint8_t xDot, yDot;

            case UP_KEY:
                xDot = xPos;
                yDot = (yPos - BUTTON_RADIUS + DOT_RADIUS);
                Paint_DrawCircle(xDot, yDot, DOT_RADIUS, 0x07FD, DOT_PIXEL_1X1, DRAW_FILL_FULL);
                break;
            case DOWN_KEY:
                xDot = xPos;
                yDot = (yPos + BUTTON_RADIUS - DOT_RADIUS);
                Paint_DrawCircle(xDot, yDot, DOT_RADIUS, 0x07FD, DOT_PIXEL_1X1, DRAW_FILL_FULL);
                break;
            case LEFT_KEY:
                xDot = (xPos - BUTTON_RADIUS + DOT_RADIUS);
                yDot = yPos;
                Paint_DrawCircle(xDot, yDot, DOT_RADIUS, 0x07FD, DOT_PIXEL_1X1, DRAW_FILL_FULL);
                break;
            case RIGHT_KEY:
                xDot = (xPos + BUTTON_RADIUS - DOT_RADIUS);
                yDot = yPos;
                Paint_DrawCircle(xDot, yDot, DOT_RADIUS, 0x07FD, DOT_PIXEL_1X1, DRAW_FILL_FULL);
                break;
            case OK_KEY:
                Paint_DrawCircle(xPos, yPos, DOT_RADIUS, 0x07FD, DOT_PIXEL_1X1, DRAW_FILL_FULL);
                break;
            case OTHER_KEY:
        }
    }
}