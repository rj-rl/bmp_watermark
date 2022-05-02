add_library(
    subsample-algo
    src/subsample_algo.h
    src/subsample_algo.cpp
    )

target_include_directories(
    subsample-algo
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
)
