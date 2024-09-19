#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"

#include "wallet_file/wallet_file.h"
#include "utils/key_print_utils.h"


uint8_t walletBytes[256];
uint8_t passwordBytes[PASSWORD_LENGTH];


void convert_password() {
    int srcLen = strlen(FILE_PASSWORD);
    for(int dst = 0, src = 0; dst < PASSWORD_LENGTH; ++dst) {
        passwordBytes[dst] = FILE_PASSWORD[src];
        if(++src >= srcLen) {
            src = 0;
        }
    }
}

void main() {
    uint8_t address[128];
    int addressLen;
    HDWallet wallet;
    HDWallet debugWallet;
    ExtendedKey derivedKey1;
    ExtendedKey derivedKey2;
    ExtendedKey derivedKey3;
    ExtendedKey derivedKey4;

    stdio_init_all();

    // Truncate/expand defined password. This is just temporary until a proper UI is in place
    convert_password();

    init_new_wallet(&debugWallet, FILE_PASSWORD, 0, 0);

    wallet_error wErr = load_wallet_bytes(walletBytes);
    if(wErr != NO_ERROR) {
        if(GET_WF_RESULT(wErr) == WF_FILE_NOT_FOUND) {
            // Need to create a new wallet
            init_new_wallet(&wallet, FILE_PASSWORD, 0, 0);
            wErr = save_wallet(&wallet);
            if(wErr != NO_ERROR) {
                // Something bad happened. SD card might be corrupted or removed password
                printf("Error saving new wallet file. SD card may be corrupt\n");
            }
        } else {
            // Something bad happened. SD card might be corrupted
            printf("Error loading wallet file. SD card may be corrupt\n");
        }
    } else {
        // Data loaded, revive key
        memcpy(wallet.password, passwordBytes, PASSWORD_LENGTH);
        wErr = rehydrate_wallet(&wallet, walletBytes);
        if(wErr != NO_ERROR) {
            // Something bad happened. File might be corrupted or password
            // was incorrect
            switch(wErr) {
                case WF_INVALID_PASSWORD:
                    printf("Error decrypting wallet file. Password incorrect\n");
                    break;

                case WF_WALLET_FILE_CORRUPTED:
                    printf("Error decrypting wallet file. File corrupt\n");
                    break;

                default:
                    printf("Error decrypting wallet file. Unknown error: 0x%04X\n", wErr);
                    break;
            }
        }
    }

    if(wErr == NO_ERROR) {
        print_key_details("Master Key", &wallet.masterKey);
        print_key_details("Debug Key", &debugWallet.masterKey);
    }

    // derive_child_key(&wallet.masterKey, 44, &derivedKey1);
    // print_key_details("m/44'", &derivedKey1);

    // derive_child_key(&derivedKey1, 0, &derivedKey2);
    // print_key_details("m/44'/0'", &derivedKey2);

    // derive_child_key(&derivedKey2, 0, &derivedKey3);
    // print_key_details("m/44'/0'/0'", &derivedKey3);

    // derive_child_key(&derivedKey2, 1, &derivedKey4);
    // print_key_details("m/44'/0'/1'", &derivedKey4);

    while(true) {
        sleep_ms(500);
    }
}
