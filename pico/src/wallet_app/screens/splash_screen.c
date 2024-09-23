#include "wallet_app/screens/wallet_screen.h"
#include "waveshare_lcd/lib/GUI/GUI_Paint.h"


extern const unsigned char WALLET_LOGO[];


void splash_screen_entered();
void splash_screen_key_pressed(DisplayKey key);
void splash_screen_key_released(DisplayKey key);
void splash_screen_key_held(DisplayKey key);
void draw_splash_screen();


WalletScreen gSplashScreen = {
    .screenID = SPLASH_SCREEN,
    .keyPressFunction = splash_screen_key_pressed,
    .keyReleaseFunction = splash_screen_key_released,
    .keyHoldFunction = splash_screen_key_held,
    .screenEnterFunction = splash_screen_entered,
    .screenUpdateFunction = NULL,
    .drawFunction = draw_splash_screen,
    .screenExitFunction = NULL,
    .screenData = NULL
};


void splash_screen_entered() {
    gSplashScreen.userInteractionCompleted = false;
}

void splash_screen_key_pressed(DisplayKey key) {
}

void splash_screen_key_released(DisplayKey key) {
    gSplashScreen.userInteractionCompleted = true;
}

void splash_screen_key_held(DisplayKey key) {
}

void draw_splash_screen() {
    Paint_DrawImage(WALLET_LOGO, 0, 0, 128, 128);
}
