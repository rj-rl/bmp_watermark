#pragma once

#include <byte.h>
#include <bmp.h>
#include <yuv.h>
#include <subsample_algo.h>
#include <matrix.h>

#include <vector>
#include <utility>
#include <stddef.h>

// converts a BMP into YUV444p
YUV BMP_to_YUV444(const BMP& bmp);

// converts a BMP into YUV420p, either sequentially or in parallel
YUV BMP_to_YUV420(const BMP& bmp, bool do_run_in_parallel = true);

// converts a YUV444p into YUV420p using custom sampling method
template<typename TSampler>
YUV YUV444_to_YUV420(const YUV& src, TSampler sampler)
{
    using namespace std;
    using namespace Utility;
    using namespace Sample;

    const auto width = src.width;
    const auto height = src.height;
    const auto image_size_px = width * height;
    // number of chroma subsamples
    const auto chroma_sub_count = chroma_count_420(width, height);

    vector<byte_t> yuv420_data(image_size_px + chroma_sub_count);
    auto dst_Cb = begin(yuv420_data) + image_size_px;
    auto dst_Cr = dst_Cb + chroma_sub_count / 2;

    auto src_Cb = begin(src.data) + image_size_px;
    auto src_Cr = src_Cb + image_size_px;

    // luminance remains at full resolution
    copy(begin(src.data), src_Cb, begin(yuv420_data));

    // interpret source Cb/Cr values as matrices for convenience
    const Matrix src_Cb_mat{src_Cb, width, height};
    const Matrix src_Cr_mat{src_Cr, width, height};
    size_t row = 0u;
    size_t col = 0u;

    while (row < height) {
        *dst_Cb++ = sample(src_Cb_mat, row, col, sampler);
        *dst_Cr++ = sample(src_Cr_mat, row, col, sampler);

        col = (col + 2) % width;
        // skip every other line (first check for even width, second for odd)
        if (col == 0 || col == 1) {
            col = 0; row += 2;
        }
    }

    return YUV{move(yuv420_data), width, height, YUV::Type::Planar420};
}
