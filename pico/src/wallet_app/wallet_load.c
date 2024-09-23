#include "wallet_load.h"
#include "wallet_defs.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>


void initial_app_state_update(WalletLoadStateController* controller);
void load_wallet_file_state_update(WalletLoadStateController* controller);
void get_password_for_load_state_update(WalletLoadStateController* controller);
void display_password_error_for_load_state_update(WalletLoadStateController* controller);
void create_wallet_file_state_update(WalletLoadStateController* controller);
void get_password_for_create_state_update(WalletLoadStateController* controller);


extern WalletScreen gPasswordEntryScreen;
extern WalletScreen gInfoMessageScreen;
extern WalletScreen gTimedInfoMessageScreen;
extern WalletScreen gIconMessageScreen;


void display_icon_message_screen(WalletLoadStateController* controller, IconType iconType, const char* format, ...) {
    void* writePtr = controller->screenDataBuffer;

    memcpy(writePtr, &iconType, sizeof(IconType));
    writePtr += sizeof(IconType);

    va_list args;
    va_start(args, format);
    vsprintf(writePtr, format, args);
    va_end(args);

    controller->currentScreen = &gIconMessageScreen;
    gIconMessageScreen.screenEnterFunction(controller->screenDataBuffer);
}

void display_timed_info_message_screen(WalletLoadStateController* controller, uint16_t timeoutMS, const char* format, ...) {
    void* writePtr = controller->screenDataBuffer;

    memcpy(writePtr, &timeoutMS, sizeof(uint16_t));
    writePtr += sizeof(uint16_t);

    va_list args;
    va_start(args, format);
    vsprintf(writePtr, format, args);
    va_end(args);

    controller->currentScreen = &gTimedInfoMessageScreen;
    gTimedInfoMessageScreen.screenEnterFunction(controller->screenDataBuffer);
}


void init_wallet_load_state_controller(WalletLoadStateController* controller) {
    controller->currentState = PW_INITIAL_APP_STATE;
    display_timed_info_message_screen(controller, 1000, "Loading wallet");
}

void update_wallet_load_state_controller(WalletLoadStateController* controller) {
    if(controller->currentScreen && controller->currentScreen->screenUpdateFunction) {
        controller->currentScreen->screenUpdateFunction();
    }

    switch(controller->currentState) {
        case PW_INITIAL_APP_STATE:
            initial_app_state_update(controller);
            break;
        case PW_LOAD_WALLET_FILE_STATE:
            load_wallet_file_state_update(controller);
            break;
        case PW_GET_PASSWORD_FOR_LOAD_STATE:
            get_password_for_load_state_update(controller);
            break;
        case PW_DISPLAY_PASSWORD_ERROR_FOR_LOAD_STATE:
            display_password_error_for_load_state_update(controller);
            break;
        case PW_CREATE_WALLET_FILE_STATE:
            create_wallet_file_state_update(controller);
            break;
        case PW_GET_PASSWORD_FOR_CREATE_STATE:
            get_password_for_create_state_update(controller);
            break;
        case PW_TERMINAL_ERROR_STATE:
            break;
        case PW_WALLET_READY_STATE:
            break;
    }
}

void initial_app_state_update(WalletLoadStateController* controller) {
    // Initial state, transition to load wallet state
    if(controller->currentScreen->userInteractionCompleted) {
        controller->currentState = PW_LOAD_WALLET_FILE_STATE;
    }
}

void load_wallet_file_state_update(WalletLoadStateController* controller) {
    wallet_error err = load_wallet_bytes(controller->walletFileBuffer);

    if(NO_ERROR == err) {
        // Wallet data loaded, need to get password to unlock
        controller->currentState = PW_GET_PASSWORD_FOR_LOAD_STATE;
        
        gPasswordEntryScreen.screenEnterFunction(controller->screenDataBuffer);
        controller->currentScreen = &gPasswordEntryScreen;
    } else if(GET_WF_RESULT(err) == WF_FILE_NOT_FOUND) {
        // There was no wallet file, need to create a new file
        controller->currentState = PW_CREATE_WALLET_FILE_STATE;
        gInfoMessageScreen.screenEnterFunction("No wallet file\n\nCreating new\nwallet");
        controller->currentScreen = &gInfoMessageScreen;
    } else {
        // Loading wallet failed completely, display error state
        display_icon_message_screen(
            controller, ERROR, 
            "Load failed\nCode: 0x%04X", err
        );

        controller->walletLoadError = err;
        controller->currentState = PW_TERMINAL_ERROR_STATE;
    }
}

void get_password_for_load_state_update(WalletLoadStateController* controller) {
    wallet_error decryptError;

    assert(controller->currentScreen->screenID == PASSWORD_ENTRY_SCREEN);

    if(controller->currentScreen->userInteractionCompleted) {
        // Get password bytes from screen
        controller->currentScreen->screenExitFunction(controller->wallet.password);

        // Check file data can be decrypted
        decryptError = rehydrate_wallet(&controller->wallet, controller->walletFileBuffer);
        if(decryptError != NO_ERROR) {
            // Something bad happened. File might be corrupted or password
            // was incorrect
            switch(GET_WF_RESULT(decryptError)) {
                case WF_INVALID_PASSWORD:
                    // Password was incorrect
                    controller->currentState = PW_DISPLAY_PASSWORD_ERROR_FOR_LOAD_STATE;
                    gInfoMessageScreen.screenEnterFunction("Incorrect password\n\nPlease try again.");
                    controller->currentScreen = &gInfoMessageScreen;
                    break;
                default:
                    // Fundamental, irrecoverable error
                    display_icon_message_screen(
                        controller, ERROR, 
                        "Error decrypting\nCode: 0x%04X", decryptError
                    );

                    controller->walletLoadError = decryptError;
                    controller->currentState = PW_TERMINAL_ERROR_STATE;
                    break;
            }
        } else {
            // All is good - file bytes have been decrypted into wallet instance
            display_icon_message_screen(
                controller, SUCCESS, 
                "Wallet loaded!"
            );
            controller->currentState = PW_WALLET_READY_STATE;
        }
    }
}

void display_password_error_for_load_state_update(WalletLoadStateController* controller) {
    assert(controller->currentScreen->screenID == INFO_MESSAGE_SCREEN);

    if(controller->currentScreen->userInteractionCompleted) {
        controller->currentState = PW_GET_PASSWORD_FOR_LOAD_STATE;
        
        gPasswordEntryScreen.screenEnterFunction(controller->screenDataBuffer);
        controller->currentScreen = &gPasswordEntryScreen;
    }
}

void create_wallet_file_state_update(WalletLoadStateController* controller) {
    assert(controller->currentScreen->screenID == INFO_MESSAGE_SCREEN);

    if(controller->currentScreen->userInteractionCompleted) {
        controller->currentState = PW_GET_PASSWORD_FOR_CREATE_STATE;
        
        gPasswordEntryScreen.screenEnterFunction(controller->screenDataBuffer);
        controller->currentScreen = &gPasswordEntryScreen;
    }
}

void get_password_for_create_state_update(WalletLoadStateController* controller) {
    uint8_t userPasswordBytes[USER_PASSWORD_LENGTH];
    wallet_error saveError;

    assert(controller->currentScreen->screenID == PASSWORD_ENTRY_SCREEN);

    if(controller->currentScreen->userInteractionCompleted) {
        // Get password bytes from screen
        controller->currentScreen->screenExitFunction(userPasswordBytes);

        // Need to create a new wallet
        init_new_wallet(&controller->wallet, userPasswordBytes, 0, 0);
        saveError = save_wallet(&controller->wallet);
        if(saveError != NO_ERROR) {
            // Something bad happened. SD card might be corrupted or removed
            display_icon_message_screen(
                controller, ERROR, 
                "Error saving\nCode: 0x%04X", saveError
            );

            controller->walletLoadError = saveError;
            controller->currentState = PW_TERMINAL_ERROR_STATE;
        } else {
            // All is good - new wallet instance is ready and data has been stored and encrypted on disk
            display_icon_message_screen(
                controller, SUCCESS, 
                "Wallet loaded!"
            );
            controller->currentState = PW_WALLET_READY_STATE;
        }
    }
}

bool is_wallet_load_complete(WalletLoadStateController* controller) {
    return (
        (controller->currentState == PW_WALLET_READY_STATE) ||
        (controller->currentState == PW_TERMINAL_ERROR_STATE)
    );
}
