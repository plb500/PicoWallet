#include "wallet_screen.h"
#include  "gfx/gfx_utils.h"


#define BUTTON_RADIUS           (BUTTON_HEIGHT / 2)
#define DOT_RADIUS              (3)
#define NUM_BUTTONS             (4)

static const WalletPaintColor ICON_COLOR = PW_TEAL;

void draw_dot_icon(int8_t xDir, int8_t yDir, uint16_t xPos, uint16_t yPos, uint8_t largeRadius, uint8_t smallRadius, uint16_t color) {
    uint8_t xDot = xPos;
    uint8_t yDot = yPos;

    if(xDir > 0 ) {
        xDot += (BUTTON_RADIUS - DOT_RADIUS);
    } else if(xDir < 0) {
        xDot -= (BUTTON_RADIUS - DOT_RADIUS);
    }

    if(yDir > 0 ) {
        yDot += (BUTTON_RADIUS - DOT_RADIUS);
    } else if(yDir < 0) {
        yDot -= (BUTTON_RADIUS - DOT_RADIUS);
    }

    wallet_gfx_draw_circle(xPos, yPos, BUTTON_RADIUS, 1, false, color);
    wallet_gfx_draw_circle(xDot, yDot, DOT_RADIUS, 1, true, color);
}

void draw_mnemonic_icon(uint16_t xPos, uint16_t yPos, uint8_t size, uint16_t color) {
    const uint8_t NUM_LINES = 4;
    uint16_t x1 = xPos, y1 = yPos;
    uint16_t x2 = (x1 + size), y2 = (yPos + size);
    uint16_t vSpacing = size / (NUM_LINES + 1);
    uint16_t hSpacing = size / (6);
    uint16_t lineLength = (size - (hSpacing * 3)) / 2;

    // Draw outline
    wallet_gfx_draw_rectangle(x1, y1, x2, y2, 1, false, color);

    // Draw lines
    for(int i = 0; i < NUM_LINES; ++i) {
        x1 = (xPos + hSpacing);
        x2 = x1 + lineLength;
        y1 = (yPos + (vSpacing * (i + 1)));
        y2 = y1;
        wallet_gfx_draw_line(x1, y1, x2, y2, 1, color);
        x1 = x2 + hSpacing;
        x2 = x1 + lineLength;
        wallet_gfx_draw_line(x1, y1, x2, y2, 1, color);
    }
}

void render_button_bar(const KeyButtonType types[]) {
    const WalletDisplayInfo* displayInfo = get_display_info();

    const uint16_t centerSpacingH = (displayInfo->displayWidth / NUM_BUTTONS);
    
    uint8_t vPadding = 2;

    int yPos = displayInfo->displayHeight - BUTTON_RADIUS - vPadding;
    
    for(int i = 0, xPos = (centerSpacingH / 2); i < NUM_BUTTONS; ++i, xPos += centerSpacingH) {
        uint16_t x1, y1, x2, y2;

        if(types[i] == NO_KEY) {
            continue;
        }

        switch(types[i]) {
            case UP_KEY:
                draw_dot_icon(0, -1, xPos, yPos, BUTTON_RADIUS, DOT_RADIUS, ICON_COLOR);
                break;
            case DOWN_KEY:
                draw_dot_icon(0, 1, xPos, yPos, BUTTON_RADIUS, DOT_RADIUS, ICON_COLOR);
                break;
            case LEFT_KEY:
                draw_dot_icon(-1, 0, xPos, yPos, BUTTON_RADIUS, DOT_RADIUS, ICON_COLOR);
                break;
            case RIGHT_KEY:
                draw_dot_icon(1, 0, xPos, yPos, BUTTON_RADIUS, DOT_RADIUS, ICON_COLOR);
                break;
            case MNEMONIC_KEY:
                draw_mnemonic_icon((xPos - BUTTON_RADIUS), (yPos - BUTTON_RADIUS), (BUTTON_RADIUS * 2), ICON_COLOR);
                break;
            case OK_KEY:
                draw_dot_icon(0, 0, xPos, yPos, BUTTON_RADIUS, DOT_RADIUS, ICON_COLOR);
                break;
            case OTHER_KEY:
        }
    }
}
