#include <stdio.h>
#include "pico/stdlib.h"

#include "wallet_file/wallet_file.h"
#include "utils/key_print_utils.h"

const char* KEY_HEADER                  = "<#8BitVault_OK!>"; 
#define KEY_HEADER_SIZE     (16)


void main() {
    uint8_t address[128];
    int addressLen;
    HDWallet wallet;
    ExtendedKey derivedKey1;
    ExtendedKey derivedKey2;
    ExtendedKey derivedKey3;
    ExtendedKey derivedKey4;

    stdio_init_all();

    init_new_wallet(&wallet, 0, 0);
    print_key_details("Master Key", &wallet.masterKey);

    derive_child_key(&wallet.masterKey, 44, &derivedKey1);
    print_key_details("m/44'", &derivedKey1);

    derive_child_key(&derivedKey1, 0, &derivedKey2);
    print_key_details("m/44'/0'", &derivedKey2);

    derive_child_key(&derivedKey2, 0, &derivedKey3);
    print_key_details("m/44'/0'/0'", &derivedKey3);

    derive_child_key(&derivedKey2, 1, &derivedKey4);
    print_key_details("m/44'/0'/1'", &derivedKey4);


    while(true) {
        sleep_ms(500);
    }
}
