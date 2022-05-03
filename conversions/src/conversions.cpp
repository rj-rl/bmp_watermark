#include <conversions.h>
#include <bmp.h>

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

struct YCbCr_px {
    byte_t Y;
    byte_t Cb;
    byte_t Cr;
};

// converts an RGB pixel into a YCbCr pixel
YCbCr_px RGB_to_YCbCr_px(RGB_px rgb_px)
{
    // transform the channels
    uint16_t Y = TV_matrix[0] * rgb_px.R
        + TV_matrix[1] * rgb_px.G
        + TV_matrix[2] * rgb_px.B;

    int16_t Cb = TV_matrix[3] * rgb_px.R
        + TV_matrix[4] * rgb_px.G
        + TV_matrix[5] * rgb_px.B;

    int16_t Cr = TV_matrix[6] * rgb_px.R
        + TV_matrix[7] * rgb_px.G
        + TV_matrix[8] * rgb_px.B;

    // scale down to 8 bit
    Y = (Y + 128) >> 8;
    Cb = (Cb + 128) >> 8;
    Cr = (Cr + 128) >> 8;
    // offset
    Y = Y + 16;
    Cb = Cb + 128;
    Cr = Cr + 128;
    // sanity check
    assert(Cb > 0 && Cr > 0);

    return YCbCr_px{static_cast<byte_t>(Y),
                    static_cast<byte_t>(Cb),
                    static_cast<byte_t>(Cr)};
}

vector<byte_t> BMP_to_YUV444(const BMP& bmp)
{
    const auto& RGB_data = bmp.pixel_data;

    vector<byte_t> YUV_data(RGB_data.size() * bmp.info_header.bit_count / 8);
    size_t image_size_px = bmp.width() * bmp.height();

    for (size_t i = 0; i < image_size_px; ++i) {
        // BGR channel order is assumed
        auto [Y, Cb, Cr] = RGB_to_YCbCr_px(RGB_data[i]);
        // luminance plane
        YUV_data[i] = Y;
        // chrominance planes
        YUV_data[image_size_px + i] = Cb;
        YUV_data[2 * image_size_px + i] = Cr;
    }
    return YUV_data;
}

size_t calc_chroma_count_420(size_t width, size_t height)
{
    size_t image_size_px = width * height;
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

