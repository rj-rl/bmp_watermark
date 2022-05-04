#pragma once

#include <byte.h>
#include <bmp.h>
#include <yuv.h>
#include <matrix.h>

#include <vector>
#include <utility>
#include <stddef.h>

YUV BMP_to_YUV444(const BMP& bmp);

// returns the number of chroma samples needed for an image
// of given width and height using 4:2:0 scheme
size_t chroma_count_420(size_t width, size_t height);

YUV BMP_to_YUV420(const BMP& bmp);

template<typename TCallable>
YUV YUV444_to_YUV420(const YUV& src, TCallable subsample)
{
    using namespace std;
    using namespace Utility;

    const auto width = src.width;
    const auto height = src.height;
    const auto image_size_px = width * height;
    // number of chroma subsamples
    const auto chroma_sub_count = chroma_count_420(width, height);

    vector<byte_t> yuv420_data(image_size_px + chroma_sub_count);
    auto dst_Cb_begin = begin(yuv420_data) + image_size_px;
    auto dst_Cr_begin = dst_Cb_begin + chroma_sub_count / 2;

    auto src_Cb_begin = begin(src.data) + image_size_px;
    auto src_Cr_begin = src_Cb_begin + image_size_px;

    // luminance remains at full resolution
    copy(begin(src.data), src_Cb_begin, begin(yuv420_data));

    // interpret source Cb/Cr values as matrices for convenience
    const Matrix src_Cb_mat{src_Cb_begin, width, height};
    const Matrix src_Cr_mat{src_Cr_begin, width, height};
    size_t row = 0u;
    size_t col = 0u;

    while (row < height) {
        *dst_Cb_begin++ = subsample(src_Cb_mat, row, col);
        *dst_Cr_begin++ = subsample(src_Cr_mat, row, col);

        col = (col + 2) % width;
        // skip every other line (first check for even width, second for odd)
        if (col == 0 || col == 1) {
            col = 0; row += 2;
        }
    }

    return YUV{move(yuv420_data), width, height, YUV::Type::Planar420};
}
