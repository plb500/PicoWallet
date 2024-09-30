#ifndef _GFX_UTILS_H_
#define _GFX_UTILS_H_

#include "wallet_app/screens/wallet_screen.h"


typedef enum {
    PW_WHITE    = 0xFFFF,
    PW_BLACK    = 0x0000,
    PW_GREEN    = 0x07E0,
    PW_TEAL     = 0x07FD,
    PW_MAGENTA  = 0xF81F
} WalletPaintColor;

typedef struct {
    uint16_t displayWidth;
    uint16_t displayHeight;
} WalletDisplayInfo;

typedef struct {
    const uint8_t charWidth;        // Width of a single character glyph, in bits
    const uint8_t charHeight;       // Height of a single character glyph, in bits
    const uint8_t numChars;         // Number of glyphs contained in the set
    const uint8_t glyphs[];         // Underlying glyph data
} WalletFont;


// Setup functions
int init_display();
const WalletDisplayInfo* get_display_info();
void update_display(WalletScreen* screen);
void shutdown_display();


// Drawing interface functions
void wallet_gfx_clear_display(WalletPaintColor color);
void wallet_gfx_draw_char(uint16_t xPos, uint16_t yPos, char c, const WalletFont* font, WalletPaintColor foregroundColor, WalletPaintColor backgroundColor);
void wallet_gfx_draw_string(uint16_t xPos, uint16_t yPos, const char* string, uint16_t stringLen, const WalletFont* font, WalletPaintColor foregroundColor, WalletPaintColor backgroundColor);
void wallet_gfx_draw_bitmap(const uint8_t* bitmap, uint16_t xPos, uint16_t yPos, uint16_t bitmapWidth, uint16_t bitmapHeight);
void wallet_gfx_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t lineWidth, WalletPaintColor color);
void wallet_gfx_draw_circle(uint16_t centerX, uint16_t centerY, uint16_t radius, uint8_t lineWidth, bool filled, WalletPaintColor color);
void wallet_gfx_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t lineWidth, bool filled, WalletPaintColor color);


#endif      // _GFX_UTILS_H_
