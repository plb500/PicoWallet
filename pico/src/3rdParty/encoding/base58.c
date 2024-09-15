#include "base58.h"
#include <string.h>

static const char b58digits_ordered[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

int base58_encode(uint8_t *input, int inputLen, uint8_t *output) {
    int carry;
    size_t i, j, high, zcount = 0;
    size_t size;
    while (zcount < inputLen && !input[zcount])
        ++zcount;

    size = (inputLen - zcount) * 138 / 100 + 1;
    uint8_t buf[size];
    memset(buf, 0, size);

    for (i = zcount, high = size - 1; i < inputLen; ++i, high = j)
    {
        for (carry = input[i], j = size - 1; (j > high) || carry; --j)
        {
            carry += 256 * buf[j];
            buf[j] = carry % 58;
            carry /= 58;
        }
    }

    for (j = 0; j < size && !buf[j]; ++j);

    if (zcount)
        memset(output, '1', zcount);
    for (i = zcount; j < size; ++i, ++j)
        output[i] = b58digits_ordered[buf[j]];
    output[i] = '\0';

    return (i + 1);
}
