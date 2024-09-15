#ifndef _KEY_UTILS_H_
#define _KEY_UTILS_H_

#define PRIVATE_KEY_LENGTH                  (32)
#define PUBLIC_KEY_LENGTH                   (33)
#define CHAIN_CODE_LENGTH                   (32)
#define FINGERPRINT_LENGTH                  (4)

#include "pico/stdlib.h"


#define QR_CODE_SIZE                        (29)
#define QR_CODE_BYTES                       (106)

typedef enum {
    BTC_MAIN_NET    = 0x00,
    BTC_TEST_NET    = 0x6F
} BTCNetwork;


typedef struct {
    uint8_t privateKey[PRIVATE_KEY_LENGTH + 1];
    uint8_t chainCode[CHAIN_CODE_LENGTH];
    uint8_t publicKey[PUBLIC_KEY_LENGTH];
    uint8_t depth;
    uint8_t fingerprint[FINGERPRINT_LENGTH];
    uint8_t parentFingerprint[FINGERPRINT_LENGTH];
    uint32_t index;
} ExtendedKey;


/**
 * Generate a new master key.
 * 
 * mnemonic         in      The string that may be used when generating the seed phrase. Optional.
 * mnemonicLen      in      The number of characters in the "mnemonic" parameter
 * dest             out     Storage for the newly created key     
 */
int generate_master_key(const uint8_t *mnemonic, int mnemonicLen, ExtendedKey* dest);

/**
 * Derive a new key from the supplied parent key.
 * 
 * NB: This function derives hardened children *only*, although the index parameters
 * still start at 0 (the index offsets are applied internally within this function)
 * 
 * parentKey        in      The parent key from which to derive the new key. 
 * index            in      The child index of the newly created key within the parent.
 * dest             out     Storage for the newly created key     
 */
int derive_child_key(const ExtendedKey* parentKey, uint32_t index, ExtendedKey* dest);


// Address utilities
int get_extended_private_key_address(const ExtendedKey* key, uint8_t* address);
int get_extended_public_key_address(const ExtendedKey* key, uint8_t* address);
int get_p2pkh_public_address(const ExtendedKey* key, uint8_t* address);
int get_p2wpkh_public_address(const ExtendedKey* key, uint8_t* address);
int get_private_key_wif(const ExtendedKey* key, BTCNetwork network, uint8_t* address);

int get_private_key_wif_qr(const ExtendedKey* key, BTCNetwork network, uint8_t* qrcode);
int get_p2pkh_qr(const ExtendedKey* key, uint8_t* qrcode);


#endif      // _KEY_UTILS_H_
