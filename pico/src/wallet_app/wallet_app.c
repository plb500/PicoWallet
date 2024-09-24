#include "wallet_app.h"
#include "screens/splash_screen.h"
#include "wallet_load.h"
#include "gfx/gfx_utils.h"


#define HOLD_REPEAT_TIME_MS                 (250)

typedef enum {
    APP_SPLASH_SCREEN,
    APP_LOADING_WALLET,
    APP_USING_WALLET
} ApplicationState;


static const uint8_t KEY_PINS[] = {
    15,
    17,
    2,
    3
};

static uint8_t keyStates[NUM_KEYS];
static uint32_t keyHoldTimes[NUM_KEYS];

static WalletScreen currentScreen;
static ApplicationState currentAppState;
static WalletLoadStateController walletLoadStateController = {
    .currentScreen = &currentScreen
};


WalletScreen* get_current_screen() {
    switch(currentAppState) {
        case APP_SPLASH_SCREEN:
            return &currentScreen;
        case APP_LOADING_WALLET:
            return walletLoadStateController.currentScreen;
        case APP_USING_WALLET:
            return NULL;
    }
}

void do_splash_screen_update() {
    if(currentScreen.exitCode) {
        init_wallet_load_state_controller(&walletLoadStateController);
        currentAppState = APP_LOADING_WALLET;
    }
}

void do_wallet_load_update() {
    update_wallet_load_state_controller(&walletLoadStateController);
}

void init_key_buttons() {
    for(int i = 0; i < NUM_KEYS; ++i) {
        gpio_init(KEY_PINS[i]);
        gpio_pull_up(KEY_PINS[i]);
        gpio_set_dir(KEY_PINS[i], GPIO_IN);
        keyStates[i] = 1;
    }
}

void update_buttons(WalletScreen* currentScreen) {
    for(int i = 0; i < NUM_KEYS; ++i) {
        uint8_t currentState = gpio_get(KEY_PINS[i]);
        bool stateChange = (currentState != keyStates[i]);

        if(stateChange) {
            if(currentState) {
                // Released
                if(currentScreen->keyReleaseFunction) {
                    currentScreen->keyReleaseFunction(currentScreen, i);
                }
            } else {
                // Pressed
                if(currentScreen->keyPressFunction) {
                    currentScreen->keyPressFunction(currentScreen, i);
                }
                keyHoldTimes[i] = to_ms_since_boot(get_absolute_time());
            }
        } else if(!currentState) {
            // Check for hold
            uint32_t currentTime = to_ms_since_boot(get_absolute_time());
            if((currentTime - keyHoldTimes[i]) > HOLD_REPEAT_TIME_MS) {
                if(currentScreen->keyHoldFunction) {
                    currentScreen->keyHoldFunction(currentScreen, i);
                }
                keyHoldTimes[i] = currentTime;
            }
        }

        keyStates[i] = currentState;
    }
}

void init_application() {
    init_key_buttons();
    init_display();

    currentAppState = APP_SPLASH_SCREEN;
    init_splash_screen(&currentScreen);
}

void update_application() {
    update_buttons(get_current_screen());
    update_display(get_current_screen());

    switch(currentAppState) {
        case APP_SPLASH_SCREEN:
            do_splash_screen_update();
            break;
        case APP_LOADING_WALLET:
            do_wallet_load_update();
            break;
        case APP_USING_WALLET:
            break;
    }
}

void shutdown_application() {
    // Nothing required, currently
}
