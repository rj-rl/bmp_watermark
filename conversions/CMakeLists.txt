add_library(
    conversions
    src/conversions.h
    src/simd_conversions.h
    src/conversions.cpp
    )

target_include_directories(
    conversions
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/bmp/src
    ${CMAKE_SOURCE_DIR}/yuv/src
    ${CMAKE_SOURCE_DIR}/subsample-algo/src
)

target_compile_options(conversions PRIVATE -march=native)

find_package(Threads)
target_link_libraries(conversions PUBLIC bmp yuv ${CMAKE_THREAD_LIBS_INIT})
