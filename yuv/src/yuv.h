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
        Planar420
    };

    size_t width  = 0u;
    size_t height = 0u;
    Type type = Type::Planar420;
    std::vector<Utility::byte_t> data;

    YUV(size_t width, size_t height, Type type);

    static YUV from_file(const std::string& filename,
                         size_t width, size_t height, Type type);

private:
    YUV(const std::string& filename, size_t width, size_t height, Type type);
};
