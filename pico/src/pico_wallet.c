#include <stdio.h>
#include "pico/stdlib.h"

#include "wallet_file/wallet_file.h"
#include "qrcode/qrcode.h"


const char* KEY_HEADER                  = "<#8BitVault_OK!>"; 
#define KEY_HEADER_SIZE     (16)


void print_qr_code(const uint8_t* address) {
    // The structure to manage the QR code
    QRCode qrcode;

    // Allocate a chunk of memory to store the QR code
    uint8_t qrcodeBytes[qrcode_getBufferSize(3)];

   qrcode_initText(&qrcode, qrcodeBytes, 3, ECC_LOW, address);

    printf("\u2588\u2588\u2588\u2588");
    for (uint8_t x = 0; x < qrcode.size; x++) {
        printf("\u2588\u2588");
    }
    printf("\u2588\u2588\u2588\u2588\n");

    printf("\u2588\u2588  ");
    for (uint8_t x = 0; x < qrcode.size; x++) {
        printf("  ");
    }
    printf("  \u2588\u2588\n");

    for (uint8_t y = 0; y < qrcode.size; y++) {
        printf("\u2588\u2588  ");
        for (uint8_t x = 0; x < qrcode.size; x++) {
            if (qrcode_getModule(&qrcode, x, y)) {
                printf("\u2588\u2588");
            } else {
                printf("  ");
            }
        }
        printf("  \u2588\u2588");
        printf("\n");
    }

    printf("\u2588\u2588  ");
    for (uint8_t x = 0; x < qrcode.size; x++) {
        printf("  ");
    }
    printf("  \u2588\u2588\n");

    printf("\u2588\u2588\u2588\u2588");
    for (uint8_t x = 0; x < qrcode.size; x++) {
        printf("\u2588\u2588");
    }
    printf("\u2588\u2588\u2588\u2588\n");
}


void debug_key(const char* title, const ExtendedKey* key) {
    int addressLen;
    uint8_t address[128];

    printf("Extended key - %s\n\n", title);

    printf(" Private key bytes: ");
    for(int i = 0; i < PRIVATE_KEY_LENGTH; ++i) {
        printf("%02X", key->privateKey[i]);
    }
    printf("\n");

    printf("  Chain code bytes: ");
    for(int i = 0; i < CHAIN_CODE_LENGTH; ++i) {
        printf("%02X", key->chainCode[i]);
    }
    printf("\n");

    printf("  Public key bytes: ");
    for(int i = 0; i < PUBLIC_KEY_LENGTH; ++i) {
        printf("%02X", key->publicKey[i]);
    }
    printf("\n\n");

    printf("   Private address: ");
    addressLen = get_extended_private_key_address(key, address);
    for(int i = 0; i < addressLen; ++i) {
        printf("%c", address[i]);
    }
    printf("\n");
    printf("         +     WIF: ");
    addressLen = get_private_key_wif(key, BTC_MAIN_NET, address);
    printf("%s", address);
    printf("\n");
    
    printf("         + QR Code: \n\n");
    print_qr_code(address);
    printf("\n\n");

    printf("    Public address: ");
    addressLen = get_extended_public_key_address(key, address);
    for(int i = 0; i < addressLen; ++i) {
        printf("%c", address[i]);
    }
    printf("\n");
    printf("          +  P2PKH: ");
    addressLen = get_p2pkh_public_address(key, address);
    for(int i = 0; i < addressLen; ++i) {
        printf("%c", address[i]);
    }
    printf("\n");

    printf("          + P2WPKH: ");
    addressLen = get_p2wpkh_public_address(key, address);
    for(int i = 0; i < addressLen; ++i) {
        printf("%c", address[i]);
    }
    printf("\n\n");


    printf("       Fingerprint: ");
    for(int i = 0; i < FINGERPRINT_LENGTH; ++i) {
        printf("%02X", key->fingerprint[i]);
    }
    printf("\n");

    printf("Parent Fingerprint: ");
    for(int i = 0; i < FINGERPRINT_LENGTH; ++i) {
        printf("%02X", key->parentFingerprint[i]);
    }
    printf("\n");

    printf("             Index: 0x%08X\n", key->index);
    printf("\n            ************\n\n");
}


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
    debug_key("Master Key", &wallet.masterKey);

    derive_child_key(&wallet.masterKey, 44, &derivedKey1);
    debug_key("m/44'", &derivedKey1);

    derive_child_key(&derivedKey1, 0, &derivedKey2);
    debug_key("m/44'/0'", &derivedKey2);

    derive_child_key(&derivedKey2, 0, &derivedKey3);
    debug_key("m/44'/0'/0'", &derivedKey3);

    derive_child_key(&derivedKey2, 1, &derivedKey4);
    debug_key("m/44'/0'/1'", &derivedKey4);


    while(true) {
        sleep_ms(500);
    }
}
