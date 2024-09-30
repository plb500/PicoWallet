#include "wallet_load.h"
#include "wallet_defs.h"

#include "utils/wallet_file.h"
#include "screens/password_entry_screen.h"
#include "screens/info_message_screen.h"
#include "screens/timed_info_message_screen.h"
#include "screens/icon_message_screen.h"
#include "screens/mnemonic_display_screen.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>


#define MESSAGE_LEN         (48)
static char displayMessage[MESSAGE_LEN];
static const KeyButtonType NO_KEYS[] = {
    NO_KEY,
    NO_KEY,
    NO_KEY,
    NO_KEY
};

static const KeyButtonType WALLET_CREATED_KEYS[] = {
    MNEMONIC_KEY,
    NO_KEY,
    NO_KEY,
    OK_KEY
};


void initial_app_state_update(WalletLoadStateController* controller);

void load_wallet_state_update(WalletLoadStateController* controller);
void recover_wallet_state_update(WalletLoadStateController* controller);
void create_wallet_state_update(WalletLoadStateController* controller);

void get_password_for_load_state_update(WalletLoadStateController* controller);
void get_password_for_create_state_update(WalletLoadStateController* controller);
void display_password_error_for_load_state_update(WalletLoadStateController* controller);

void wallet_ready_state_update(WalletLoadStateController* controller);
void display_mnemonic_state_update(WalletLoadStateController* controller);


void display_icon_message_screen(WalletLoadStateController* controller, IconType iconType, const KeyButtonType buttons[NUM_KEYS], const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(displayMessage, MESSAGE_LEN, format, args);
    va_end(args);

    IconMessageScreenData data = {
        .iconType = iconType,
        .message = displayMessage,
    };
    memcpy(data.buttonKeys, buttons, NUM_KEYS);

    init_icon_message_screen(controller->currentScreen, data);
    controller->currentScreen->screenEnterFunction(controller->currentScreen);
}

void display_info_message_screen(WalletLoadStateController* controller, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(displayMessage, MESSAGE_LEN, format, args);
    va_end(args);

    InfoMessageScreenData data = {
        .message = displayMessage
    };

    init_info_message_screen(controller->currentScreen, data);
    controller->currentScreen->screenEnterFunction(controller->currentScreen);
}

void display_timed_info_message_screen(WalletLoadStateController* controller, uint16_t timeoutMS, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(displayMessage, MESSAGE_LEN, format, args);
    va_end(args);

    TimedInfoMessageScreenData data = {
        .timeoutMS = timeoutMS,
        .message = displayMessage
    };

    init_timed_info_message_screen(controller->currentScreen, data);
    controller->currentScreen->screenEnterFunction(controller->currentScreen);
}


void init_wallet_load_state_controller(WalletLoadStateController* controller) {
    controller->currentState = PW_INITIAL_APP_STATE;
    controller->userExitRequested = false;
    display_timed_info_message_screen(controller, 100, "Loading wallet");
}

void update_wallet_load_state_controller(WalletLoadStateController* controller) {
    if(controller->currentScreen && controller->currentScreen->screenUpdateFunction) {
        controller->currentScreen->screenUpdateFunction(controller->currentScreen);
    }

    switch(controller->currentState) {
        case PW_INITIAL_APP_STATE:
            initial_app_state_update(controller);
            break;
        case PW_LOAD_WALLET_STATE:
            load_wallet_state_update(controller);
            break;
        case PW_CREATE_WALLET_STATE:
            create_wallet_state_update(controller);
            break;
        case PW_RECOVER_WALLET_STATE:
            recover_wallet_state_update(controller);
            break;
        case PW_GET_PASSWORD_FOR_DECRYPT_STATE:
            get_password_for_load_state_update(controller);
            break;
        case PW_GET_PASSWORD_FOR_ENCRYPT_STATE:
            get_password_for_create_state_update(controller);
            break;
        case PW_DISPLAY_PASSWORD_ERROR_FOR_LOAD_STATE:
            display_password_error_for_load_state_update(controller);
            break;
        case PW_DISPLAY_CREATED_MNEMONIC_STATE:
            display_mnemonic_state_update(controller);
            break;
        case PW_TERMINAL_ERROR_STATE:
            break;
        case PW_WALLET_READY_STATE:
            wallet_ready_state_update(controller);
            break;
    }
}

void initial_app_state_update(WalletLoadStateController* controller) {
    // Initial state, transition to load wallet state
    if(controller->currentScreen->exitCode) {
        controller->currentState = PW_LOAD_WALLET_STATE;
    }
}

void load_wallet_state_update(WalletLoadStateController* controller) {
    wallet_error err = load_wallet_data_from_disk(controller->walletFileBuffer);

    if(NO_ERROR == err) {
        // Wallet data loaded, need to get password to decrypt
        controller->currentState = PW_GET_PASSWORD_FOR_DECRYPT_STATE;
        
        init_password_entry_screen(controller->currentScreen);
        controller->currentScreen->screenEnterFunction(controller->currentScreen);
    } else if(GET_WF_RESULT(err) == WF_FILE_NOT_FOUND) {
        // There was no wallet file, attempt recovery
        controller->currentState = PW_RECOVER_WALLET_STATE;
        display_info_message_screen(controller, "No wallet file\n\nLooking for\nrecovery file");
    } else { 
        // Loading wallet failed completely, display error state
        display_icon_message_screen(
            controller, ERROR, NO_KEYS,
            "Load failed\nCode: 0x%04X", err
        );

        controller->walletLoadError = err;
        controller->currentState = PW_TERMINAL_ERROR_STATE;
    }
}

void recover_wallet_state_update(WalletLoadStateController* controller) {
    assert(controller->currentScreen->screenID == INFO_MESSAGE_SCREEN);

    if(controller->currentScreen->exitCode) {
        wallet_error err = recover_wallet(controller->wallet);
        if(NO_ERROR == err) {
            // Wallet recovered, need to get password to encrypt new wallet
            controller->currentState = PW_GET_PASSWORD_FOR_ENCRYPT_STATE;
            
            init_password_entry_screen(controller->currentScreen);
            controller->currentScreen->screenEnterFunction(controller->currentScreen);
        } else if(GET_WF_RESULT(err) == WF_FILE_NOT_FOUND) {
            // There was no wallet file, create brand new wallet
            controller->currentState = PW_CREATE_WALLET_STATE;
            display_info_message_screen(controller, "No recovery file\n\nCreating new\nwallet");
        } else { 
            // Loading wallet failed completely, display error state
            display_icon_message_screen(
                controller, ERROR, NO_KEYS,
                "Load failed\nCode: 0x%04X", err
            );

            controller->walletLoadError = err;
            controller->currentState = PW_TERMINAL_ERROR_STATE;
        }
    }
}

void create_wallet_state_update(WalletLoadStateController* controller) {
    assert(controller->currentScreen->screenID == INFO_MESSAGE_SCREEN);
    
    if(controller->currentScreen->exitCode) {
        init_new_wallet(controller->wallet, 0, 0, 0);
        controller->currentState = PW_GET_PASSWORD_FOR_ENCRYPT_STATE;
        
        init_password_entry_screen(controller->currentScreen);
        controller->currentScreen->screenEnterFunction(controller->currentScreen);
    }
}

void get_password_for_create_state_update(WalletLoadStateController* controller) {
    uint8_t userPasswordBytes[USER_PASSWORD_LENGTH];
    wallet_error saveError;

    assert(controller->currentScreen->screenID == PASSWORD_ENTRY_SCREEN);

    if(controller->currentScreen->exitCode) {
        // Get password bytes from screen
        controller->currentScreen->screenExitFunction(controller->currentScreen, userPasswordBytes);

        // Set wallet password
        set_wallet_password(controller->wallet, userPasswordBytes);

        // Save new wallet to disk
        saveError = save_wallet(controller->wallet);
        if(saveError != NO_ERROR) {
            // Something bad happened. SD card might be corrupted or removed
            display_icon_message_screen(
                controller, ERROR, NO_KEYS,
                "Error saving\nCode: 0x%04X", saveError
            );

            controller->walletLoadError = saveError;
            controller->currentState = PW_TERMINAL_ERROR_STATE;
        } else {
            // All is good - new wallet instance is ready and data has been stored and encrypted on disk
            display_icon_message_screen(
                controller, SUCCESS, WALLET_CREATED_KEYS,
                "Wallet ready!"
            );
            controller->currentState = PW_WALLET_READY_STATE;
        }
    }
}

void get_password_for_load_state_update(WalletLoadStateController* controller) {
    wallet_error decryptError;

    assert(controller->currentScreen->screenID == PASSWORD_ENTRY_SCREEN);

    if(controller->currentScreen->exitCode) {
        // Get password bytes from screen
        controller->currentScreen->screenExitFunction(controller->currentScreen, controller->wallet->password);

        // Check file data can be decrypted
        decryptError = decrypt_wallet_data(controller->walletFileBuffer, controller->wallet);
        if(decryptError != NO_ERROR) {
            // Something bad happened. File might be corrupted or password
            // was incorrect
            switch(GET_WF_RESULT(decryptError)) {
                case WF_INVALID_PASSWORD:
                    // Password was incorrect
                    controller->currentState = PW_DISPLAY_PASSWORD_ERROR_FOR_LOAD_STATE;
                    display_info_message_screen(controller, "Incorrect password\n\nPlease try again.");
                    break;
                default:
                    // Fundamental, irrecoverable error
                    display_icon_message_screen(
                        controller, ERROR, NO_KEYS,
                        "Error decrypting\nCode: 0x%04X", decryptError
                    );

                    controller->walletLoadError = decryptError;
                    controller->currentState = PW_TERMINAL_ERROR_STATE;
                    break;
            }
        } else {
            // All is good - file bytes have been decrypted into wallet instance
            display_icon_message_screen(
                controller, SUCCESS, WALLET_CREATED_KEYS,
                "Wallet ready!"
            );
            controller->currentState = PW_WALLET_READY_STATE;
        }
    }
}

void display_password_error_for_load_state_update(WalletLoadStateController* controller) {
    assert(controller->currentScreen->screenID == INFO_MESSAGE_SCREEN);

    if(controller->currentScreen->exitCode) {
        controller->currentState = PW_GET_PASSWORD_FOR_DECRYPT_STATE;
        
        init_password_entry_screen(controller->currentScreen);
        controller->currentScreen->screenEnterFunction(controller->currentScreen);
    }
}

void wallet_ready_state_update(WalletLoadStateController* controller) {
    if(controller->currentScreen->exitCode == MNEMONIC_KEY) {
        MnemonicMessageScreenData data;
        for(int i = 0; i < MNEMONIC_LENGTH; ++i) {
            data.mnemonicSentence[i] = controller->wallet->mnemonicSentence[i];
        }
        controller->currentState = PW_DISPLAY_CREATED_MNEMONIC_STATE;
        
        init_mnemonic_display_screen(controller->currentScreen, data);
        controller->currentScreen->screenEnterFunction(controller->currentScreen);
    } else {
        controller->userExitRequested = (controller->currentScreen->exitCode > 0);
    }
}

void display_mnemonic_state_update(WalletLoadStateController* controller) {
    if(controller->currentScreen->exitCode) {
        display_icon_message_screen(
            controller, SUCCESS, WALLET_CREATED_KEYS,
            "Wallet ready!"
        );
        controller->currentState = PW_WALLET_READY_STATE;
    }
}

bool is_wallet_load_complete(WalletLoadStateController* controller) {
    return (
        (controller->currentState == PW_WALLET_READY_STATE) ||
        (controller->currentState == PW_TERMINAL_ERROR_STATE)
    );
}
