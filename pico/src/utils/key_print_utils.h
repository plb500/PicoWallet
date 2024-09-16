#ifndef _KEY_PRINT_UTILS_H_
#define _KEY_PRINT_UTILS_H_

#include "key_utils.h"

void print_key_qr_codes(const ExtendedKey* key);
void print_key_details(const char* title, const ExtendedKey* key);
void print_bytes(const uint8_t* a, int aSize, int formatted);

#endif      // _KEY_PRINT_UTILS_H_
