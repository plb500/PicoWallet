aux_source_directory(. DIR_GUI_SRCS)

add_library(GUI ${DIR_GUI_SRCS})

target_include_directories(GUI PRIVATE ../Config)

target_link_libraries(GUI PUBLIC Config)