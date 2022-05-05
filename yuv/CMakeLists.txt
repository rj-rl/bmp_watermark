add_library(
    yuv
    src/yuv.h
    src/yuv.cpp
    )

target_include_directories(
    yuv
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/conversions/src
)

target_link_libraries(yuv PUBLIC conversions)
