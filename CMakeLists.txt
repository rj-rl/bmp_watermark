cmake_minimum_required(VERSION 3.16)
project(bmp_overlay VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
# don't use gnu++ extensions
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-Wall -Werror)

# debug defines
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        add_compile_definitions(_LIBCPP_DEBUG=1)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_definitions(_GLIBCXX_DEBUG _GLIBCXX_DEBUG_PEDANTIC)
    endif()
endif()

add_subdirectory(bmp)
add_subdirectory(conversions)
add_subdirectory(subsample-algo)

add_executable(watermark main.cpp)
add_executable(sanitized_watermark main.cpp)

set(LIBS bmp conversions subsample-algo)

target_link_libraries(watermark PRIVATE ${LIBS})
#target_link_libraries(watermark PRIVATE conversions)
#target_link_libraries(watermark PRIVATE subsample-algo)
target_link_libraries(sanitized_watermark PRIVATE ${LIBS})
#target_link_libraries(sanitized_watermark PRIVATE conversions)
#target_link_libraries(sanitized_watermark PRIVATE subsample-algo)

# sanitizers
set(
    SANITIZE_OPTIONS
    -fsanitize=address,undefined 
    -fno-omit-frame-pointer 
    -fno-optimize-sibling-calls
    )

target_link_options(sanitized_watermark PUBLIC ${SANITIZE_OPTIONS})
target_compile_options(sanitized_watermark PUBLIC ${SANITIZE_OPTIONS})
