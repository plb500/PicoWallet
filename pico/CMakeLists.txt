cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(myproject_test C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(WALLET_SRC "${PROJECT_SOURCE_DIR}/src")

pico_sdk_init()

add_subdirectory(${WALLET_SRC}/3rdParty/FatFs_SPI)
add_subdirectory(${WALLET_SRC}/3rdParty/waveshare_lcd)

add_executable(PicoWallet
    ${WALLET_SRC}/3rdParty/cryptography/cifra/aes.c
    ${WALLET_SRC}/3rdParty/cryptography/cifra/blockwise.c
    ${WALLET_SRC}/3rdParty/cryptography/cifra/chash.c
    ${WALLET_SRC}/3rdParty/cryptography/cifra/hmac.c
    ${WALLET_SRC}/3rdParty/cryptography/cifra/pbkdf2.c
    ${WALLET_SRC}/3rdParty/cryptography/cifra/sha512.c
    ${WALLET_SRC}/3rdParty/cryptography/cifra/sha256.c
    ${WALLET_SRC}/3rdParty/cryptography/uECC/uECC.c
    ${WALLET_SRC}/3rdParty/encoding/base58.c
    ${WALLET_SRC}/3rdParty/encoding/bech32.c
    ${WALLET_SRC}/3rdParty/hashing/ripemd160.c
    ${WALLET_SRC}/3rdParty/qrcode/qrcode.c

    ${WALLET_SRC}/gfx/gfx_utils.c

    ${WALLET_SRC}/utils/big_int/big_int.c
    ${WALLET_SRC}/utils/bip39_wordlist.c
    ${WALLET_SRC}/utils/platform/wallet_random.c
    ${WALLET_SRC}/utils/hash_utils.c
    ${WALLET_SRC}/utils/key_print_utils.c
    ${WALLET_SRC}/utils/key_utils.c
    ${WALLET_SRC}/utils/seed_utils.c

    ${WALLET_SRC}/wallet_app/hd_wallet.c
    ${WALLET_SRC}/wallet_app/wallet_app.c
    ${WALLET_SRC}/wallet_app/wallet_load.c

    ${WALLET_SRC}/wallet_app/screens/icon_message_screen.c
    ${WALLET_SRC}/wallet_app/screens/info_message_screen.c
    ${WALLET_SRC}/wallet_app/screens/timed_info_message_screen.c
    ${WALLET_SRC}/wallet_app/screens/password_entry_screen.c
    ${WALLET_SRC}/wallet_app/screens/splash_screen.c
    ${WALLET_SRC}/wallet_app/screens/wallet_screen.c

    ${WALLET_SRC}/wallet_app/screens/images/wallet_logo.c
    ${WALLET_SRC}/wallet_app/screens/images/icons.c

    ${WALLET_SRC}/pico_wallet.c
)

target_link_libraries(PicoWallet
    pico_stdlib
    pico_rand
    FatFs_SPI
    WaveshareLCD

    hardware_pwm
    hardware_spi
)

target_include_directories(PicoWallet PRIVATE 
    ${WALLET_SRC}
    ${WALLET_SRC}/3rdParty
    ${WALLET_SRC}/3rdParty/cryptography/cifra/
    ${WALLET_SRC}/3rdParty/cryptography/cifra/ext
)

target_compile_definitions(PicoWallet PRIVATE 
    DEBUG_SEED_GENERATION=1
    USE_DEBUG_ENTROPY=1

    # CHANGE THESE TO YOUR OWN PEPPER/PASSWORD
    PASSCODE_PEPPER="<~8BitVault_OK!>"
    FILE_PASSWORD="password"
)


pico_enable_stdio_usb(PicoWallet 1)
pico_enable_stdio_uart(PicoWallet 0)

pico_add_extra_outputs(PicoWallet)
