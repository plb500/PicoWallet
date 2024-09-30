add_library(cifra INTERFACE)

target_sources(cifra INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/aes.c
    ${CMAKE_CURRENT_LIST_DIR}/blockwise.c
    ${CMAKE_CURRENT_LIST_DIR}/chash.c
    ${CMAKE_CURRENT_LIST_DIR}/hmac.c
    ${CMAKE_CURRENT_LIST_DIR}/pbkdf2.c
    ${CMAKE_CURRENT_LIST_DIR}/sha512.c
    ${CMAKE_CURRENT_LIST_DIR}/sha256.c
)

target_include_directories(cifra INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(cifra INTERFACE
    pico_stdlib
)
