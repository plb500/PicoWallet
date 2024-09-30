cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(PicoWallet C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(WALLET_SRC "${PROJECT_SOURCE_DIR}/src")

pico_sdk_init()

# Generate the password pepper
find_package(Python COMPONENTS Interpreter REQUIRED)
execute_process(
    COMMAND ${Python_EXECUTABLE} "pepper_gen.py" "-l 16" 
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    OUTPUT_VARIABLE PEPPER
)

add_subdirectory(${WALLET_SRC}/3rdParty/FatFs_SPI)
add_subdirectory(${WALLET_SRC}/3rdParty/waveshare_lcd)
add_subdirectory(${WALLET_SRC}/3rdParty/cryptography/cifra)

add_executable(PicoWallet
    ${WALLET_SRC}/3rdParty/cryptography/uECC/uECC.c
    ${WALLET_SRC}/3rdParty/encoding/base58.c
    ${WALLET_SRC}/3rdParty/encoding/bech32.c
    ${WALLET_SRC}/3rdParty/hashing/ripemd160.c
    ${WALLET_SRC}/3rdParty/qrcode/qrcode.c

    ${WALLET_SRC}/gfx/waveshare_gfx_interface.c
    ${WALLET_SRC}/gfx/wallet_fonts.c

    ${WALLET_SRC}/utils/big_int/big_int.c
    ${WALLET_SRC}/utils/bip39_wordlist.c
    ${WALLET_SRC}/utils/platform/wallet_random.c
    ${WALLET_SRC}/utils/hash_utils.c
    ${WALLET_SRC}/utils/key_print_utils.c
    ${WALLET_SRC}/utils/key_utils.c
    ${WALLET_SRC}/utils/seed_utils.c
    ${WALLET_SRC}/utils/wallet_file.c

    ${WALLET_SRC}/wallet_app/hd_wallet.c
    ${WALLET_SRC}/wallet_app/wallet_app.c
    ${WALLET_SRC}/wallet_app/wallet_load.c
    ${WALLET_SRC}/wallet_app/wallet_browse.c

    ${WALLET_SRC}/wallet_app/screens/icon_message_screen.c
    ${WALLET_SRC}/wallet_app/screens/info_message_screen.c
    ${WALLET_SRC}/wallet_app/screens/mnemonic_display_screen.c
    ${WALLET_SRC}/wallet_app/screens/timed_info_message_screen.c
    ${WALLET_SRC}/wallet_app/screens/password_entry_screen.c
    ${WALLET_SRC}/wallet_app/screens/qr_code_screen.c
    ${WALLET_SRC}/wallet_app/screens/splash_screen.c
    ${WALLET_SRC}/wallet_app/screens/wallet_navigate_screen.c
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
    cifra

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
    PASSCODE_PEPPER="${PEPPER}"
)

pico_enable_stdio_usb(PicoWallet 1)
pico_enable_stdio_uart(PicoWallet 0)

pico_add_extra_outputs(PicoWallet)
