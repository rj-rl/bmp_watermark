#include <conversions.h>
#include <subsample_algo.h>
#include <bmp.h>
#include <yuv.h>

#include <algorithm>
#include <cmath>
#include <cassert>

using namespace std;
using namespace Utility;

/*
    TV levels (aka studio swing):
        Y in [16, 235] and Cb, Cr in [16, 240]

    PC levels (aka full swing):
        Y, Cb, Cr in [0, 255]
*/

static const vector<int32_t> TV_matrix = {
     66,  129,   25,
    -38,  -74,  112,
    112,  -94,  -18
};

static const vector<int32_t> PC_matrix = {
     77,  150,   29,
    -43,  -84,  127,
    127, -106,  -21
};

// =================== various utility functions & types ===================== //

size_t chroma_count_420(size_t width, size_t height)
{
    auto image_size_px = width * height;
    // exactly how many samples of chrominance we need depends on
    // height and width being odd or even
    bool is_height_even = (height % 2 == 0);
    bool is_width_even = (width % 2 == 0);

    if (is_height_even && is_width_even) {
        return image_size_px / 2;
    }
    else if (!is_height_even && !is_width_even) {
        return (image_size_px + width + height + 1) / 2;
    }
    else if (!is_height_even) {
        return (image_size_px + height) / 2;
    }
    else {
        return (image_size_px + width) / 2;
    }
}

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
    uint16_t Y = TV_matrix[0] * rgb_px.R
        + TV_matrix[1] * rgb_px.G
        + TV_matrix[2] * rgb_px.B;
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
    int16_t Cb = TV_matrix[3] * rgb_px.R
        + TV_matrix[4] * rgb_px.G
        + TV_matrix[5] * rgb_px.B;

    int16_t Cr = TV_matrix[6] * rgb_px.R
        + TV_matrix[7] * rgb_px.G
        + TV_matrix[8] * rgb_px.B;
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

YUV BMP_to_YUV420(const BMP& bmp)
{
    const auto width = bmp.width();
    const auto height = bmp.height();
    const auto image_size_px = width * height;
    // number of chroma subsamples
    const auto chroma_sub_count = chroma_count_420(width, height);

    vector<byte_t> yuv_data(image_size_px + chroma_sub_count);
    auto dst_Y_begin = begin(yuv_data);
    auto dst_Cb_begin = dst_Y_begin + image_size_px;
    auto dst_Cr_begin = dst_Cb_begin + chroma_sub_count / 2;

    const Matrix<const RGB_px> RGB_mat{begin(bmp.data), width, height};

    for (size_t row = 0u; row < height; ++row) {
        for (size_t col = 0u; col < width; ++col) {
            // fill luma plane
            *dst_Y_begin++ = RGB_to_Y(RGB_mat(row, col));

            // sample every other pixel in every other row
            if (row % 2 == 0 && col % 2 == 0) {
                // calculate mean RGB values
                RGB_px mean = mean_RGB_px(Subsample::select(RGB_mat, row, col));
                auto [Cb, Cr] = RGB_to_CbCr(mean);
                // fill chrominance planes
                *dst_Cb_begin++ = Cb;
                *dst_Cr_begin++ = Cr;
            }
        }
    }
    return YUV{move(yuv_data), width, height, YUV::Type::Planar420};
}
