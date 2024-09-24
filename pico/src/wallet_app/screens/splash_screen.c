#include "splash_screen.h"
#include "waveshare_lcd/lib/GUI/GUI_Paint.h"


extern const unsigned char WALLET_LOGO[];


void splash_screen_entered(WalletScreen* screen);
void splash_screen_key_pressed(WalletScreen* screen, DisplayKey key);
void splash_screen_key_released(WalletScreen* screen, DisplayKey key);
void splash_screen_key_held(WalletScreen* screen, DisplayKey key);
void draw_splash_screen(WalletScreen* screen);


void init_splash_screen(WalletScreen* screen) {
    screen->screenID = SPLASH_SCREEN;
    screen->keyPressFunction = splash_screen_key_pressed;
    screen->keyReleaseFunction = splash_screen_key_released;
    screen->keyHoldFunction = splash_screen_key_held;
    screen->screenEnterFunction = splash_screen_entered;
    screen->screenUpdateFunction = NULL;
    screen->drawFunction = draw_splash_screen;
    screen->screenExitFunction = NULL;
}


void splash_screen_entered(WalletScreen* screen) {
    screen->exitCode = 0;
}

void splash_screen_key_pressed(WalletScreen* screen, DisplayKey key) {
}

void splash_screen_key_released(WalletScreen* screen, DisplayKey key) {
    screen->exitCode = 1;
}

void splash_screen_key_held(WalletScreen* screen, DisplayKey key) {
}

void draw_splash_screen(WalletScreen* screen) {
    Paint_DrawImage(WALLET_LOGO, 0, 0, 128, 128);
}
