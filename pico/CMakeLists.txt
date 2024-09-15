cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(myproject_test C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(WALLET_SRC "${PROJECT_SOURCE_DIR}/src")

pico_sdk_init()

add_executable(PicoWallet
    ${WALLET_SRC}/3rdParty/cryptography/cifra/src/blockwise.c
    ${WALLET_SRC}/3rdParty/cryptography/cifra/src/chash.c
    ${WALLET_SRC}/3rdParty/cryptography/cifra/src/hmac.c
    ${WALLET_SRC}/3rdParty/cryptography/cifra/src/pbkdf2.c
    ${WALLET_SRC}/3rdParty/cryptography/cifra/src/sha512.c
    ${WALLET_SRC}/3rdParty/cryptography/cifra/src/sha256.c
    ${WALLET_SRC}/3rdParty/cryptography/uECC/uECC.c
    ${WALLET_SRC}/3rdParty/encoding/base58.c
    ${WALLET_SRC}/3rdParty/encoding/bech32.c
    ${WALLET_SRC}/3rdParty/hashing/ripemd160.c
    ${WALLET_SRC}/3rdParty/QRCode/src/qrcode.c
    ${WALLET_SRC}/utils/platform/wallet_random.c

    ${WALLET_SRC}/utils/big_int/big_int.c
    ${WALLET_SRC}/utils/bip39_wordlist.c
    ${WALLET_SRC}/utils/hash_utils.c
    ${WALLET_SRC}/utils/seed_utils.c

    ${WALLET_SRC}/wallet_file/wallet_file.c

    ${WALLET_SRC}/pico_wallet.c
)

target_link_libraries(PicoWallet
    pico_stdlib
    pico_rand
)

target_include_directories(PicoWallet PRIVATE 
    ${WALLET_SRC}
    ${WALLET_SRC}/3rdParty
    ${WALLET_SRC}/3rdParty/cryptography/cifra/src
    ${WALLET_SRC}/3rdParty/cryptography/cifra/src/ext
)

target_compile_definitions(PicoWallet PRIVATE 
    DEBUG_SEED_GENERATION=1
    USE_DEBUG_ENTROPY=1
)


pico_enable_stdio_usb(PicoWallet 0)
pico_enable_stdio_uart(PicoWallet 1)

pico_add_extra_outputs(PicoWallet)
