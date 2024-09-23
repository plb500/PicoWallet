#include "pico/stdlib.h"

#include "wallet_app/wallet_app.h"


void main() {
    stdio_init_all();

    init_application();

    while(true) {
        update_application();
    }
}
