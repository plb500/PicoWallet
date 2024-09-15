#include "key_print_utils.h"
#include <stdio.h>


uint8_t PRIVATE_KEY_QR_BUFFER[QR_CODE_BYTES];
uint8_t PUBLIC_ADDRESS_QR_BUFFER[QR_CODE_BYTES];

uint8_t PRIVATE_KEY_PRINT_BUFFER[128];
uint8_t PUBLIC_ADDRESS_PRINT_BUFFER[64];


void print_key_qr_codes(const ExtendedKey* key) {
    uint16_t currentPrivateBit = 0;
    uint16_t currentPublicBit = 0;

    get_private_key_wif_qr(key, BTC_MAIN_NET, PRIVATE_KEY_QR_BUFFER);
    get_p2pkh_qr(key, PUBLIC_ADDRESS_QR_BUFFER);


    // Top border row
    printf("\u2588\u2588\u2588\u2588");
    for (uint8_t x = 0; x < QR_CODE_SIZE; x++) {
        printf("\u2588\u2588");
    }
    printf("\u2588\u2588\u2588\u2588    \u2588\u2588\u2588\u2588");
    for (uint8_t x = 0; x < QR_CODE_SIZE; x++) {
        printf("\u2588\u2588");
    }
    printf("\u2588\u2588\u2588\u2588\n");

    // Top padding row
    printf("\u2588\u2588  ");
    for (uint8_t x = 0; x < QR_CODE_SIZE; x++) {
        printf("  ");
    }

    printf("  \u2588\u2588    \u2588\u2588  ");     // Divider

    for (uint8_t x = 0; x < QR_CODE_SIZE; x++) {
        printf("  ");
    }
    printf("  \u2588\u2588\n");

    // QR code rows
    for (uint8_t y = 0; y < QR_CODE_SIZE; y++) {
        printf("\u2588\u2588  ");
        
        // Private key
        for (uint8_t x = 0; x < QR_CODE_SIZE; x++) {
            uint8_t cByte = currentPrivateBit / 8;
            uint8_t cBit = currentPrivateBit % 8;

            uint8_t set = (PRIVATE_KEY_QR_BUFFER[cByte] & (1 << (7 - cBit)));
            if(set != 0) {
                printf("\u2588\u2588");
            } else {
                printf("  ");
            }

            ++currentPrivateBit;
        }

        printf("  \u2588\u2588    \u2588\u2588  ");

        // Public
        for (uint8_t x = 0; x < QR_CODE_SIZE; x++) {
            uint8_t cByte = currentPublicBit / 8;
            uint8_t cBit = currentPublicBit % 8;

            uint8_t set = (PUBLIC_ADDRESS_QR_BUFFER[cByte] & (1 << (7 - cBit)));

            if(set != 0) {
                printf("\u2588\u2588");
            } else {
                printf("  ");
            }

            ++currentPublicBit;
        }

        printf("  \u2588\u2588");
        printf("\n");
    }

    // Bottom padding row
    printf("\u2588\u2588  ");
    for (uint8_t x = 0; x < QR_CODE_SIZE; x++) {
        printf("  ");
    }

    printf("  \u2588\u2588    \u2588\u2588  ");         // Divider
    
    for (uint8_t x = 0; x < QR_CODE_SIZE; x++) {
        printf("  ");
    }
    printf("  \u2588\u2588\n");

    // Bottom border row
    printf("\u2588\u2588\u2588\u2588");
    for (uint8_t x = 0; x < QR_CODE_SIZE; x++) {
        printf("\u2588\u2588");
    }
    printf("\u2588\u2588\u2588\u2588    \u2588\u2588\u2588\u2588");
    for (uint8_t x = 0; x < QR_CODE_SIZE; x++) {
        printf("\u2588\u2588");
    }
    printf("\u2588\u2588\u2588\u2588\n");
}

void print_key_details(const char* title, const ExtendedKey* key) {
    int addressLen;

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
    printf("\n");

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

    printf("             Index: 0x%08X\n\n", key->index);


    printf(" BIP32 Private Key: ");
    addressLen = get_extended_private_key_address(key, PRIVATE_KEY_PRINT_BUFFER);
    for(int i = 0; i < addressLen; ++i) {
        printf("%c", PRIVATE_KEY_PRINT_BUFFER[i]);
    }
    printf("\n");
    printf("         +     WIF: ");
    addressLen = get_private_key_wif(key, BTC_MAIN_NET, PRIVATE_KEY_PRINT_BUFFER);
    printf("%s", PRIVATE_KEY_PRINT_BUFFER);
    printf("\n");
    
    printf("  BIP32 Public Key: ");
    addressLen = get_extended_public_key_address(key, PRIVATE_KEY_PRINT_BUFFER);
    for(int i = 0; i < addressLen; ++i) {
        printf("%c", PRIVATE_KEY_PRINT_BUFFER[i]);
    }
    printf("\n");
    printf("          +  P2PKH: ");
    addressLen = get_p2pkh_public_address(key, PUBLIC_ADDRESS_PRINT_BUFFER);
    for(int i = 0; i < addressLen; ++i) {
        printf("%c", PUBLIC_ADDRESS_PRINT_BUFFER[i]);
    }
    printf("\n");

    printf("          + P2WPKH: ");
    addressLen = get_p2wpkh_public_address(key, PUBLIC_ADDRESS_PRINT_BUFFER);
    for(int i = 0; i < addressLen; ++i) {
        printf("%c", PUBLIC_ADDRESS_PRINT_BUFFER[i]);
    }
    printf("\n\n");

    printf("    QR Codes:\n\n");
    print_key_qr_codes(key);

    printf("\n\n\n");
}
