add_library(
    watermark-lib
    src/watermark.h
    src/watermark.cpp
    )

target_include_directories(
    watermark-lib
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/bmp/src
    ${CMAKE_SOURCE_DIR}/yuv/src
    ${CMAKE_SOURCE_DIR}/conversions/src
)

target_link_libraries(watermark-lib PUBLIC conversions)
