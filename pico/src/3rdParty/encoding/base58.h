#ifndef BASE58_H
#define BASE58_H

#include <stdint.h>

int base58_encode(uint8_t *input, int inputLen, uint8_t *output);

#endif      // BASE58_H
