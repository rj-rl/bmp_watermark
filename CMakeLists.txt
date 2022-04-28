cmake_minimum_required(VERSION 3.16)

project(rgb_overlay VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-Wall -Werror)

add_executable(watermark main.cpp)
add_executable(sanitized_watermark main.cpp)

target_compile_options(
    sanitized_watermark 
    PUBLIC
      -fsanitize=address,undefined
      -fno-omit-frame-pointer
    )

target_link_options(
    sanitized_watermark 
    PUBLIC
      -fsanitize=address,undefined
      -fno-omit-frame-pointer
    )

target_include_directories(
    watermark
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_include_directories(
    sanitized_watermark
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)
