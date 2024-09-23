#ifndef _WALLET_SCREEN_H_
#define _WALLET_SCREEN_H_

#include "pico/types.h"

typedef enum {
    KEY_A = 0,
    KEY_B = 1,
    KEY_C = 2,
    KEY_D = 3,

    NUM_KEYS
} DisplayKey;

typedef enum {
    ERROR,
    SUCCESS,
    INFO
} IconType;

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


typedef void (*OnKeyPressedFunction) (DisplayKey);
typedef void (*OnKeyReleasedFunction) (DisplayKey);
typedef void (*OnKeyHoldFunction) (DisplayKey);
typedef void (*OnScreenEnterFunction) (uint8_t* screenDataBuffer);
typedef void (*OnScreenExitFunction) (uint8_t* outputData);
typedef void (*OnScreenUpdateFunction) (void);
typedef void (*ScreenDrawFunction) (void);


typedef struct {
    int screenID;
    OnKeyPressedFunction keyPressFunction;
    OnKeyReleasedFunction keyReleaseFunction;
    OnKeyHoldFunction keyHoldFunction;
    OnScreenEnterFunction screenEnterFunction;
    OnScreenExitFunction screenExitFunction;
    OnScreenUpdateFunction screenUpdateFunction;
    ScreenDrawFunction drawFunction;
    void* screenData;
    bool userInteractionCompleted;
} WalletScreen;


// Utility functions
void render_button_bar(const KeyButtonType types[]);

#endif      // _WALLET_SCREEN_H_
