#include "gfx_utils.h"
#include "wallet_fonts.h"

#include "waveshare_lcd/lib/Config/DEV_Config.h"
#include "waveshare_lcd/lib/LCD/LCD_1in44.h"
#include "waveshare_lcd/lib/GUI/GUI_Paint.h"

#include <string.h>

static UWORD imageBuffer[LCD_1IN44_HEIGHT * LCD_1IN44_WIDTH * 2];
static WalletDisplayInfo waveshareDisplay;

// Font maps
static sFONT WAVESHARE_FONT_MED = {
    .table = PW_FONT_MED.glyphs,
    .Width = 7,
    .Height = 12
};

static sFONT WAVESHARE_FONT_SMALL = {
    .table = PW_FONT_SMALL.glyphs,
    .Width = 7,
    .Height = 9
};

static sFONT WAVESHARE_FONT_LARGE = {
    .table = PW_FONT_LARGE.glyphs,
    .Width = 10,
    .Height = 20
};

sFONT* to_waveshare_font(const WalletFont* font) {
    if(font == &PW_FONT_SMALL) {
        return &WAVESHARE_FONT_SMALL;
    } else if(font == &PW_FONT_MED) {
        return &WAVESHARE_FONT_MED;
    } else if(font == &PW_FONT_LARGE) {
        return &WAVESHARE_FONT_LARGE;
    } else {
        return NULL;
    }
}

DOT_PIXEL to_waveshare_line_width(uint8_t lineWidth) {
    switch(lineWidth) {
        case 0:
        case 1:
            return DOT_PIXEL_1X1;
        case 2:
            return DOT_PIXEL_2X2;
        case 3:
            return DOT_PIXEL_3X3;
        case 4:
            return DOT_PIXEL_4X4;
        case 5:
            return DOT_PIXEL_5X5;
        case 6:
            return DOT_PIXEL_6X6;
        case 7:
            return DOT_PIXEL_7X7;
        case 8:
        default:
            return DOT_PIXEL_8X8;
    }
}

void start_paint();
void end_paint();

int init_display() {
    if(DEV_Module_Init()!=0){
        return -1;
    }

    LCD_1IN44_Init(HORIZONTAL);
    LCD_1IN44_Clear(BLACK);
    
    if(imageBuffer == NULL) {
        return -1;
    }

    waveshareDisplay.displayWidth = LCD_1IN44.WIDTH;
    waveshareDisplay.displayHeight = LCD_1IN44.HEIGHT;

    return 0;
}

const WalletDisplayInfo* get_display_info() {
    return &waveshareDisplay;
}

void update_display(WalletScreen* screen) {
    start_paint();

    if(screen && screen->drawFunction) {
        screen->drawFunction(screen);
    }

    end_paint();
}

void start_paint() {
    Paint_NewImage((UBYTE*)imageBuffer, LCD_1IN44.WIDTH, LCD_1IN44.HEIGHT, 0, BLACK);
    Paint_SetScale(65);
    Paint_SetRotate(ROTATE_270);
    Paint_Clear(BLACK);
}

void end_paint() {
    LCD_1IN44_Display(imageBuffer);             
}

void shutdown_display() {
    DEV_Module_Exit();
}

// Drawing interface functions
void wallet_gfx_clear_display(WalletPaintColor color) {
    Paint_Clear(color);
}

void wallet_gfx_draw_char(uint16_t xPos, uint16_t yPos, char c, const WalletFont* font, WalletPaintColor foregroundColor, WalletPaintColor backgroundColor) {
    sFONT* drawFont = to_waveshare_font(font);
    if(!drawFont) {
        return;
    }

    Paint_DrawChar(xPos, yPos, c, drawFont, backgroundColor, foregroundColor);
}

void wallet_gfx_draw_string(uint16_t xPos, uint16_t yPos, const char* string, uint16_t stringLen, const WalletFont* font, WalletPaintColor foregroundColor, WalletPaintColor backgroundColor) {
    sFONT* drawFont = to_waveshare_font(font);
    if(!drawFont) {
        return;
    }

    Paint_DrawString_EN(xPos, yPos, string, drawFont, foregroundColor, backgroundColor);
}

void wallet_gfx_draw_bitmap(const uint8_t* bitmap, uint16_t xPos, uint16_t yPos, uint16_t bitmapWidth, uint16_t bitmapHeight) {
    Paint_DrawImage(bitmap, xPos, yPos, bitmapWidth, bitmapHeight);
}

void wallet_gfx_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t lineWidth, WalletPaintColor color) {
    Paint_DrawLine(x1, y1, x2, y2, color, to_waveshare_line_width(lineWidth), LINE_STYLE_SOLID);
}

void wallet_gfx_draw_circle(uint16_t centerX, uint16_t centerY, uint16_t radius, uint8_t lineWidth, bool filled, WalletPaintColor color) {
    DRAW_FILL fill = filled ? DRAW_FILL_FULL : DRAW_FILL_EMPTY;
    Paint_DrawCircle(centerX, centerY, radius, color, to_waveshare_line_width(lineWidth), fill);
}

void wallet_gfx_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t lineWidth, bool filled, WalletPaintColor color) {
    DRAW_FILL fill = filled ? DRAW_FILL_FULL : DRAW_FILL_EMPTY;

    Paint_DrawRectangle(x1, y1, x2, y2, color, to_waveshare_line_width(lineWidth), fill);
}
