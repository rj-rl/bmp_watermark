#pragma once

#include <byte.h>
#include <bmp.h>

#include <vector>

std::vector<byte_t> BMP_to_YUV444(const BMP& bmp);

template<typename Callable>
std::vector<byte_t> BMP_to_YUV420(const BMP& bmp, Callable subsample)
{
    // TODO: bmp width is assumed to be multiple of 2, height is assumed even

    auto YUV444_data = BMP_to_YUV444(bmp);

    std::size_t image_size_px = bmp.width_px() * bmp.height_px();
    // 1 bpp for luminance, 1/2 bpp for chrominance
    std::vector<byte_t> YUV420_data(image_size_px * 3 / 2);

    auto Cb_begin = std::begin(YUV420_data) + image_size_px;
    auto Cr_begin = std::begin(YUV420_data) + image_size_px + image_size_px / 4;

    // luminance remains at full resolution
    std::copy(std::begin(YUV444_data), std::begin(YUV444_data) + image_size_px,
              std::begin(YUV420_data));

    auto subsampler = [&]() -> byte_t
    {
        static std::size_t pos = image_size_px;
        byte_t value =
            subsample(YUV444_data, bmp.width_px(), bmp.height_px(), pos);
        pos += 2;
        // skip every other line
        if (pos % bmp.width_px() == 0) {
            pos += bmp.width_px();
        }
        return value;
    };

    std::generate(Cb_begin, Cr_begin, subsampler);
    std::generate(Cr_begin, std::end(YUV420_data), subsampler);

    return YUV420_data;
}
