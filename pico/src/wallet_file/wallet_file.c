#include "wallet_file.h"


int init_new_wallet(HDWallet* wallet, const uint8_t* mnemonic, int mnemonicLen) {
    generate_master_key(mnemonic, mnemonicLen, &wallet->masterKey);

    return 1;
}

