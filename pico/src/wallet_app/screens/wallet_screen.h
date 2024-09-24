#ifndef _WALLET_SCREEN_H_
#define _WALLET_SCREEN_H_

#include "pico/types.h"


struct WalletScreen_t;
#define SCREEN_DATA_BUFFER_SIZE     (256)

typedef enum {
    KEY_A = 0,
    KEY_B = 1,
    KEY_C = 2,
    KEY_D = 3,

    NUM_KEYS
} DisplayKey;

typedef enum {
    UP_KEY,
    DOWN_KEY,
    LEFT_KEY,
    RIGHT_KEY,
    OK_KEY,
    OTHER_KEY,
    NO_KEY
} KeyButtonType;

typedef enum {
    SPLASH_SCREEN = 0,
    WALLET_LOADING_SCREEN,
    PASSWORD_ENTRY_SCREEN,
    INFO_MESSAGE_SCREEN,
    TIMED_INFO_MESSAGE_SCREEN,
    ICON_MESSAGE_SCREEN
} ScreenID;


typedef void (*OnKeyPressedFunction) (struct WalletScreen_t*, DisplayKey);
typedef void (*OnKeyReleasedFunction) (struct WalletScreen_t*, DisplayKey);
typedef void (*OnKeyHoldFunction) (struct WalletScreen_t*, DisplayKey);
typedef void (*OnScreenEnterFunction) (struct WalletScreen_t*);
typedef void (*OnScreenExitFunction) (struct WalletScreen_t*, uint8_t* outputData);
typedef void (*OnScreenUpdateFunction) (struct WalletScreen_t*);
typedef void (*ScreenDrawFunction) (struct WalletScreen_t*);


typedef struct WalletScreen_t {
    int screenID;
    OnKeyPressedFunction keyPressFunction;
    OnKeyReleasedFunction keyReleaseFunction;
    OnKeyHoldFunction keyHoldFunction;
    OnScreenEnterFunction screenEnterFunction;
    OnScreenExitFunction screenExitFunction;
    OnScreenUpdateFunction screenUpdateFunction;
    ScreenDrawFunction drawFunction;
    uint8_t screenData[SCREEN_DATA_BUFFER_SIZE];
    int exitCode;               // 0 if screen is not exiting
} WalletScreen;


// Utility functions
void render_button_bar(const KeyButtonType types[]);


#endif      // _WALLET_SCREEN_H_
