#ifndef _WALLET_FILE_H_
#define _WALLET_FILE_H_

#include "wallet_defs.h"
#include "seed_utils.h"


wallet_error load_wallet_data_from_disk(uint8_t* data);
wallet_error save_wallet_data_to_disk(const uint8_t* data);
wallet_error read_mnemonics_from_disk(char mnemonics[MNEMONIC_LENGTH][MAX_MNEMONIC_WORD_LENGTH + 1]);


#endif      // _WALLET_FILE_H_
