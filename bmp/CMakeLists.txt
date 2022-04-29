add_library(
    bmp 
    src/bmp.h 
    src/bmp.cpp
    )

target_include_directories(
    bmp
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
