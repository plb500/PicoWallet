#ifndef _BIG_INT_H_
#define _BIG_INT_H_

#include <stdint.h>


/**
 * Equivalent to (a + b) where a and b are both byte arrays representing large
 * numbers, in little-endian format
 * 
 * Returns 1 if overflow occurred, 0 otherwise
 */
int bytewise_add(const uint8_t* a, int aSize, const uint8_t* b, int bSize, uint8_t* output);

/**
 * Equivalent to (a - b) where a and b are both byte arrays representing large
 * numbers, in little-endian format
 * 
 * Returns 1 if underflow occurred, 0 otherwise
 */
int bytewise_subtract(const uint8_t* a, int aSize, const uint8_t* b, int bSize, uint8_t* output);

/**
 * Insert i into the head of array a, shifting remaining elements right. Supplied array (a) must 
 * have enough space for aSize + 1 elements
 */
int insert_and_shift(uint8_t* a, int aSize, uint8_t i);

/**
 * Numerical comparison of big integers a and b
 * 
 * Returns:
 *      1: a > b
 *      0: a == b
 *     -1: a < b
 */
int bytewise_cmp(const uint8_t* a, int aSize, const uint8_t* b, int bSize);

/**
 * Performs a modulo operation 
 * Equivalent to a % b 
 */
void bytewise_mod(const uint8_t* a, int aSize, const uint8_t* b, int bSize, uint8_t* output);

void print_bytes(const uint8_t* a, int aSize, int formatted);

#endif      // _BIG_INT_H_
