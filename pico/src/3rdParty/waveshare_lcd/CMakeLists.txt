cmake_minimum_required(VERSION 3.13)

add_library(WaveshareLCD INTERFACE) 

add_subdirectory(lib/Config)
add_subdirectory(lib/LCD)
add_subdirectory(lib/Fonts)
add_subdirectory(lib/GUI)

target_include_directories(WaveshareLCD INTERFACE
    lib/Config
    lib/GUI
    lib/LCD
    lib/Fonts
)

target_link_libraries(WaveshareLCD INTERFACE
    LCD
    GUI
    Fonts
)