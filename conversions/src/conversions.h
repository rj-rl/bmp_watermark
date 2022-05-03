#pragma once

#include <byte.h>
#include <bmp.h>
#include <matrix.h>

#include <vector>
#include <stddef.h>

std::vector<Utility::byte_t> BMP_to_YUV444(const BMP& bmp);

// returns the number of chroma samples needed for an image
// of given width and height using 4:2:0 scheme
size_t calc_chroma_count_420(size_t width, size_t height);

template<typename Callable>
std::vector<Utility::byte_t> YUV444_to_YUV420(const BMP& bmp, Callable subsample)
{
    size_t width = bmp.width();
    size_t height = bmp.height();
    size_t image_size_px = width * height;
    // number of chroma subsamples
    size_t chroma_sub_count = calc_chroma_count_420(width, height);

    std::vector<Utility::byte_t> yuv420_data(image_size_px + chroma_sub_count);
    auto yuv444_data = BMP_to_YUV444(bmp);
    
    auto dst_Cb_begin = std::begin(yuv420_data) + image_size_px;
    auto dst_Cr_begin = dst_Cb_begin + chroma_sub_count / 2;

    auto src_Cb_begin = std::begin(yuv444_data) + image_size_px;
    auto src_Cb_end = src_Cb_begin + image_size_px;
    auto src_Cr_begin = src_Cb_end;
    auto src_Cr_end = std::end(yuv444_data);

    // luminance remains at full resolution
    std::copy(std::begin(yuv444_data), src_Cb_begin, std::begin(yuv420_data));

    // interpret source Cb/Cr values as matrices for convenience
    Utility::Matrix src_Cb_mat{
        src_Cb_begin, src_Cr_begin, width, height
    };
    Utility::Matrix src_Cr_mat{
        src_Cr_begin, src_Cr_end, width, height
    };

    size_t row = 0u;
    size_t col = 0u;

    while (row < height) {
        *dst_Cb_begin++ = subsample(src_Cb_mat, row, col);
        *dst_Cr_begin++ = subsample(src_Cr_mat, row, col);

        col += 2;
        // skip every other line
        if (col % width == 0 ||  // width is even
            col % width == 1) {  // width is odd
            col = 0; row += 2;
        }
    }
    return yuv420_data;
}
