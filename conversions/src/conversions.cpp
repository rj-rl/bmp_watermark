#include <conversions.h>
#include <simd_conversions.h>
#include <subsample_algo.h>
#include <bmp.h>
#include <yuv.h>

#include <algorithm>
#include <cmath>
#include <future>
#include <thread>
#include <cassert>

using namespace std;
using namespace Utility;

namespace {  // various utility constants, functions & types
/*
    Two versions of RGB->YCbCr transformation matrices:

    TV levels (aka studio swing):
        normalizes output so that Y is in [16, 235] and Cb, Cr in [16, 240]

    PC levels (aka full swing):
        does not limit output range; Y, Cb, Cr in [0, 255]
*/
constexpr int32_t TV_matrix[] = {
     66,  129,   25,
    -38,  -74,  112,
    112,  -94,  -18
};

constexpr int32_t PC_matrix[] = {
     77,  150,   29,
    -43,  -84,  127,
    127, -106,  -21
};

// returns arithmetically mean pixel among a group of pixels
RGB_px mean_RGB_px(const vector<RGB_px>& pixels)
{
    int32_t R = 0, G = 0, B = 0;
    for (auto px : pixels) {
        R += px.R;
        G += px.G;
        B += px.B;
    }
    return RGB_px{
        .B = static_cast<byte_t>(B / pixels.size()),
        .G = static_cast<byte_t>(G / pixels.size()),
        .R = static_cast<byte_t>(R / pixels.size())
    };
}

// converts an RGB pixel into the luma component of a YCbCr pixel
byte_t RGB_to_Y(RGB_px rgb_px)
{
    uint16_t Y =
        TV_matrix[0] * rgb_px.R +
        TV_matrix[1] * rgb_px.G +
        TV_matrix[2] * rgb_px.B;
    // scale down to 8 bit
    Y = (Y + 128) >> 8;
    // offset
    return Y + 16;
}

struct Chroma {
    byte_t Cb = 0u;
    byte_t Cr = 0u;
};

// converts an RGB pixel into the two chroma components of a YCbCr pixel
Chroma RGB_to_CbCr(RGB_px rgb_px)
{
    int16_t Cb =
        TV_matrix[3] * rgb_px.R +
        TV_matrix[4] * rgb_px.G +
        TV_matrix[5] * rgb_px.B;

    int16_t Cr =
        TV_matrix[6] * rgb_px.R +
        TV_matrix[7] * rgb_px.G +
        TV_matrix[8] * rgb_px.B;
    // scale down to 8 bit
    Cb = (Cb + 128) >> 8;
    Cr = (Cr + 128) >> 8;
    // offset
    Cb = Cb + 128;
    Cr = Cr + 128;
    // sanity check
    assert(Cb > 0 && Cr > 0);
    return Chroma(Cb, Cr);
}

// converts an RGB pixel into a YCbCr pixel
YCbCr_px RGB_to_YCbCr_px(RGB_px rgb_px)
{
    auto Y = RGB_to_Y(rgb_px);
    auto [Cb, Cr] = RGB_to_CbCr(rgb_px);
    return YCbCr_px{Y, Cb, Cr};
}

// vertically divides a matrix into submatrices
template <typename T>
vector<Matrix<T>> split_into_parts(const Matrix<T>& matrix, size_t part_height)
{
    vector<Matrix<T>> parts;
    auto part_begin = begin(matrix);
    size_t row = 0u;

    while (row < matrix.height()) {
        // check if the part fits, truncate if needed
        auto current_part_height = min(part_height, matrix.height() - row);
        parts.emplace_back(part_begin, matrix.width(), current_part_height);

        row += current_part_height;
        part_begin += current_part_height * matrix.width();
    }
    return parts;
}

// slow but more flexible and a lot more easily testable;
// used as a reference
template <typename TSampler = decltype(mean_RGB_px)>
void process_part_slow(Matrix<const RGB_px> rgb_matrix,
                       byte_t* dst_Y, byte_t* dst_Cb, byte_t* dst_Cr,
                       TSampler sampler = mean_RGB_px)
{
    for (size_t row = 0u; row < rgb_matrix.height(); ++row) {
        for (size_t col = 0u; col < rgb_matrix.width(); ++col) {
            // fill luma plane
            *dst_Y++ = RGB_to_Y(rgb_matrix(row, col));
            // sample every other pixel in every other row
            if (row % 2 == 0 && col % 2 == 0) {
                // calculate mean RGB values
                auto [Cb, Cr] = RGB_to_CbCr(
                    Sample::sample(rgb_matrix, row, col, sampler)
                );
                // fill chrominance planes
                *dst_Cb++ = Cb;
                *dst_Cr++ = Cr;
            }
        }
    }
}

// processes the 'height x (16*(int)width/16)' portion using simd,
// the rest is processed manually; if 'width % 16 == 0', uses simd only
void process_part_simd(Matrix<const RGB_px> rgb_matrix,
                       byte_t* dst_Y, byte_t* dst_Cb, byte_t* dst_Cr)
{
    auto width  = rgb_matrix.width();
    auto height = rgb_matrix.height();

    // ptrs to current src pixel on top and bottom row
    auto src_top = begin(rgb_matrix);
    auto src_bot = src_top + width;
    // ptr to current pixel of destination's luma for both rows
    auto dst_Y_top = dst_Y;
    auto dst_Y_bot = dst_Y + width;

    size_t row = 0u;
    size_t col = 0u;
    // iterating until (height - 1) means next row always exists
    // same with (width - 1) and next column
    while (row < height - 1) {
        col = 0u;
        // fill 2x16 packs pixels using simd
        // (until the right edge, where pack might be <16px wide)
        while (col + 15 < width) {
            // iterate over columns and do 2 rows each time
            simd_RGB_to_YUV420(
                reinterpret_cast<const byte_t*>(src_top),
                reinterpret_cast<const byte_t*>(src_bot),
                dst_Y_top, dst_Y_bot, dst_Cb, dst_Cr
            );
            // shift all the ptrs accordingly
            col       += 16;
            src_top   += 16;
            src_bot   += 16;
            dst_Y_top += 16;
            dst_Y_bot += 16;
            dst_Cb    += 8;
            dst_Cr    += 8;
        }

        // fill the last (<16) pixels of the two rows manually
        while (col < width - 1) {
            // fill luma plane, top row
            *dst_Y_top       = RGB_to_Y(*src_top);
            *(dst_Y_top + 1) = RGB_to_Y(*(src_top + 1));
            // and bottom row
            *dst_Y_bot       = RGB_to_Y(*src_bot);
            *(dst_Y_bot + 1) = RGB_to_Y(*(src_bot + 1));
            // sample chroma at every other pixel in every other row
            // calculate mean RGB values
            uint16_t R_mean = (
                src_top->R + (src_top + 1)->R +
                src_bot->R + (src_bot + 1)->R
                ) / 4;
            uint16_t G_mean = (
                src_top->G + (src_top + 1)->G +
                src_bot->G + (src_bot + 1)->G
                ) / 4;
            uint16_t B_mean = (
                src_top->B + (src_top + 1)->B +
                src_bot->B + (src_bot + 1)->B
                ) / 4;
            // fill chrominance planes
            auto [Cb, Cr] = RGB_to_CbCr(RGB_px(B_mean, G_mean, R_mean));
            *dst_Cb = Cb;
            *dst_Cr = Cr;
            // shift all the ptrs accordingly
            col       += 2;
            src_top   += 2;
            src_bot   += 2;
            dst_Y_top += 2;
            dst_Y_bot += 2;
            dst_Cb    += 1;
            dst_Cr    += 1;
        }
        if (col == (width - 1)) {  // we only get here if width was odd
            *dst_Y_top = RGB_to_Y(*src_top);
            *dst_Y_bot = RGB_to_Y(*(src_top + width));

            auto [Cb, Cr] = RGB_to_CbCr(*src_top);
            *dst_Cb = Cb;
            *dst_Cr = Cr;
            // shift all the ptrs accordingly
            src_top   += 1;
            src_bot   += 1;
            dst_Y_top += 1;
            dst_Y_bot += 1;
            dst_Cb    += 1;
            dst_Cr    += 1;
        }

        // two rows done
        row       += 2;
        // skip bottom row in the pair, it's already done
        src_top   += width;
        src_bot   += width;
        dst_Y_top += width;
        dst_Y_bot += width;
    }

    // if the height is odd, we gotta do the last row separately
    if (height % 2 == 1) {
        col = 0u;
        while (col < width - 1) {
            *dst_Y_top       = RGB_to_Y(*src_top);
            *(dst_Y_top + 1) = RGB_to_Y(*(src_top + 1));

            if (row % 2 == 0) {
                uint16_t R_mean = (src_top->R + (src_top + 1)->R) / 2;
                uint16_t G_mean = (src_top->G + (src_top + 1)->G) / 2;
                uint16_t B_mean = (src_top->B + (src_top + 1)->B) / 2;

                auto [Cb, Cr] = RGB_to_CbCr(RGB_px(B_mean, G_mean, R_mean));
                *dst_Cb = Cb;
                *dst_Cr = Cr;
            }
            col       += 2;
            dst_Y_top += 2;
            src_top   += 2;
            dst_Cb    += 1;
            dst_Cr    += 1;
        }
        // finally do the last element, we get here if width & height were both odd
        if (col == (width - 1)) {
            auto [Y, Cb, Cr] = RGB_to_YCbCr_px(*src_top++);
            *dst_Y_top = Y;
            *dst_Cb    = Cb;
            *dst_Cr    = Cr;
        }
    }
}

}  // end of ::anonymous_namespace

// ========================= conversion functions ========================== //

YUV BMP_to_YUV444(const BMP& bmp)
{
    const auto width = bmp.width();
    const auto height = bmp.height();
    const auto image_size_px = width * height;

    vector<byte_t> yuv_data(image_size_px * 3);  // 3 bytes per pixel
    const auto& RGB_data = bmp.data;

    for (size_t i = 0; i < image_size_px; ++i) {
        auto [Y, Cb, Cr] = RGB_to_YCbCr_px(RGB_data[i]);
        // luminance plane
        yuv_data[i] = Y;
        // chrominance planes
        yuv_data[image_size_px + i] = Cb;
        yuv_data[2 * image_size_px + i] = Cr;
    }
    return YUV{move(yuv_data), width, height, YUV::Type::Planar444};
}

YUV BMP_to_YUV420(const BMP& bmp, bool do_run_in_parallel)
{
    const auto width = bmp.width();
    const auto height = bmp.height();
    const auto image_size_px = width * height;
    // number of chroma subsamples
    const auto chroma_sub_count = chroma_count_420(width, height);

    // set up the output
    vector<byte_t> yuv_data(image_size_px + chroma_sub_count);
    auto dst_Y  = &*begin(yuv_data);
    auto dst_Cb = dst_Y + image_size_px;
    auto dst_Cr = dst_Cb + chroma_sub_count / 2;

    // interpret input as a matrix
    Matrix<const RGB_px> rgb_matrix{begin(bmp.data), width, height};

    // sequential version: just process the entire matrix and return result
    if (!do_run_in_parallel) {
        process_part_simd(rgb_matrix, dst_Y, dst_Cb, dst_Cr);
        return YUV{move(yuv_data), width, height, YUV::Type::Planar420};
    }

    // parallel version: split the matrix and process parts in parallel
    const size_t concurrency = max(4u, thread::hardware_concurrency());
    auto part_height =
        max<size_t>(2, (height + (concurrency - 1)) / concurrency);
    // we want parts to be of even height for convenient 4:2:0 subsampling
    if (part_height % 2 == 1) {
        ++part_height;
    }

    const auto parts = split_into_parts(rgb_matrix, part_height);
    vector<future<void>> futures;
    for (auto part : parts) {
        futures.push_back(async(
            process_part_simd, part, dst_Y, dst_Cb, dst_Cr
        ));
        // shift the destination pointers by how much we've just filled
        // the respective planes: Y by the total number of elements in part,
        // Cb and Cr by half the part's chroma count
        dst_Y  += part.size();
        auto part_chroma_count = chroma_count_420(width, part.height()) / 2;
        dst_Cb += part_chroma_count;
        dst_Cr += part_chroma_count;
    }
    for (auto& future : futures) {
        future.get();
    }
    return YUV{move(yuv_data), width, height, YUV::Type::Planar420};
}
