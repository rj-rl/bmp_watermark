#pragma once

#include "byte.h"

#include <vector>
#include <stddef.h>

struct YCbCr_px {
    Utility::byte_t Y  = 0u;
    Utility::byte_t Cb = 0u;
    Utility::byte_t Cr = 0u;
};

struct YUV444 {
    size_t width  = 0u;
    size_t height = 0u;
    std::vector<Utility::byte_t> data;

    YUV444(size_t width, size_t height)
        : width{width}
        , height{height}
        , data(width * height * 3)
    { }
};
