#pragma once

#include <byte.h>

#include <vector>
#include <string>
#include <stddef.h>

struct YCbCr_px {
    Utility::byte_t Y  = 0u;
    Utility::byte_t Cb = 0u;
    Utility::byte_t Cr = 0u;
};

struct YUV {
    enum class Type {
        Planar444,
        Planar420,
    };

    size_t width  = 0u;
    size_t height = 0u;
    Type   type   = Type::Planar420;
    std::vector<Utility::byte_t> data;

    // ctors
    YUV(const std::string& filename,
        size_t width, size_t height, Type type);
    YUV(std::vector<Utility::byte_t> data,
        size_t width, size_t height, Type type);

    size_t frame_count() const;
};

// returns the number of chroma samples needed for an image
// of given width and height using 4:2:0 scheme
// (can't be less than 2, unless width == height == 0)
inline size_t chroma_count_420(size_t width, size_t height)
{
    // exactly how many samples of chrominance we need depends on
    // height and width being odd or even;
    // what this does is effectively 2 * ceil(width/2) * ceil(height/2)
    return 2 * ((width + 1) / 2) * ((height + 1) / 2);
}
