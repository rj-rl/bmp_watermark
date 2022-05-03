add_library(
    conversions
    src/conversions.h
    src/conversions.cpp
    )

target_include_directories(
    conversions
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/bmp/src
)

target_link_libraries(conversions PUBLIC bmp)
