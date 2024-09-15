#include "big_int.h"

#include <string.h>
#include <stdio.h>


#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))


void print_bytes(const uint8_t* a, int aSize, int formatted) {
    if(formatted) {
        printf("    ");
    } else {
        printf("0x");
    }

    for(int i = 0; i < aSize; ++i) {
        if(formatted) {
            printf("0x");
        }

        printf("%02X", a[i]);

        if(formatted) {
            if((i > 0) && (((i+1) % 8) == 0)) {
                printf("\n    ");
            } else {
                printf(" ");
            }
        }
    }
}

int bytewise_add(const uint8_t* a, int aSize, const uint8_t* b, int bSize, uint8_t* output) {
    uint16_t tmpA, tmpB, result, carry = 0;
    int limit = MAX(aSize, bSize);
    
    for(int idxA = (aSize - 1), idxB = (bSize - 1), dstIdx = (limit - 1); (idxA >= 0) || (idxB >= 0); --idxA, --idxB, --dstIdx) {
        tmpA = (idxA < 0) ? 0 : a[idxA];
        tmpB = (idxB < 0) ? 0 : b[idxB];
        result = tmpA + tmpB + carry;

        output[dstIdx] = (uint8_t) (result & 0x00FF);
        carry = result >> 8;
    }

    return carry;
}

int bytewise_subtract(const uint8_t* a, int aSize, const uint8_t* b, int bSize, uint8_t* output) {
    uint8_t tmpA, tmpB, result, borrow = 0;
    int limit = MAX(aSize, bSize);


    for(int idxA = (aSize - 1), idxB = (bSize - 1), dstIdx = (limit - 1); (idxA >= 0) || (idxB >= 0); --idxA, --idxB, --dstIdx) {
        tmpA = (idxA < 0) ? 0 : a[idxA];
        tmpB = (idxB < 0) ? 0 : b[idxB];

        uint16_t result = tmpA - tmpB - borrow;

        output[dstIdx] = (uint8_t) (result & 0x00FF);
        borrow = ((result & 0x0100) >> 8);
    }

    return borrow;
}

int insert_and_shift(uint8_t* a, int aSize, uint8_t i) {
    for(int i = aSize; i > 0; --i) {
        a[i] = a[i - 1];
    }
    a[0] = i;
}

int bytewise_cmp(const uint8_t* a, int aSize, const uint8_t* b, int bSize) {
    uint8_t tmpA, tmpB;

    int limit = MAX(aSize, bSize);
    for(int idxA = (aSize - limit), idxB = (bSize - limit); ((idxA < limit) && (idxB < limit)); ++idxA, ++idxB) {
        tmpA = (idxA < 0) ? 0 : a[idxA];
        tmpB = (idxB < 0) ? 0 : b[idxB];

        if(tmpA > tmpB) {
            return 1;
        } else if(tmpB > tmpA) {
            return -1;
        }
    }

    return 0;
}

void bytewise_mod(const uint8_t* a, int aSize, const uint8_t* b, int bSize, uint8_t* output) {
    int check = bytewise_cmp(a, aSize, b, bSize);

    if(check == -1) {
        memcpy(output, a, aSize);
        return;
    } else if(check == 0) {
        memset(output, 0, aSize);
        return;
    } else {
        memcpy(output, a, aSize);
        while(check >= 0) {
            bytewise_subtract(output, aSize, b, bSize, output);
            check = bytewise_cmp(output, aSize, b, bSize);
        }
    }
}
