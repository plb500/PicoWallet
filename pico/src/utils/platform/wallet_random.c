#include "wallet_random.h"

#include "pico/rand.h"


static unsigned char lcg_state;
static unsigned char xorshift_state;

void wallet_random_init() {
    // Nothing to do (until we want CSRNG)
}

uint8_t get_random_byte() {
    unsigned char crand = 0;
    crand = (char) (get_rand_32() & 0x000000FF);

    lcg_state = (5 * lcg_state) + 129;
    xorshift_state ^= (xorshift_state << 6);
    xorshift_state ^= (xorshift_state >> 3);
    xorshift_state ^= (xorshift_state << 5);

    return (lcg_state ^ xorshift_state ^ crand);
}
