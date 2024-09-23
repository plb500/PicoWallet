#include "gfx_utils.h"

#include "waveshare_lcd/lib/Config/DEV_Config.h"
#include "waveshare_lcd/lib/LCD/LCD_1in44.h"
#include "waveshare_lcd/lib/GUI/GUI_Paint.h"

// Frame buffer
static UWORD imageBuffer[LCD_1IN44_HEIGHT * LCD_1IN44_WIDTH * 2];

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

    return 0;
}

void update_display(WalletScreen* screen) {
    start_paint();

    if(screen && screen->drawFunction) {
        screen->drawFunction();
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
