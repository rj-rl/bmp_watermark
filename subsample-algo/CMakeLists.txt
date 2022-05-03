add_library(subsample-algo INTERFACE)
target_sources(subsample-algo INTERFACE src/subsample_algo.h)

target_include_directories(
    subsample-algo
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
)
