#include <conversions.h>
#include <bmp.h>

#include <algorithm>
#include <cmath>
#include <cassert>

using namespace std;

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

struct RGB_px {
    byte_t R;
    byte_t G;
    byte_t B;
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

    vector<byte_t> YUV_data(RGB_data.size());
    size_t image_size_px = bmp.width_px() * bmp.height_px();

    for (size_t i = 0; i < image_size_px; ++i) {
        // BGR channel order is assumed
        auto [Y, Cb, Cr] = RGB_to_YCbCr_px({
            RGB_data[3 * i + 2]  /* red */,
            RGB_data[3 * i + 1]  /* green */,
            RGB_data[3 * i]      /* blue */
        });
        // luminance plane
        YUV_data[i] = Y;
        // chrominance planes
        YUV_data[image_size_px + i] = Cb;
        YUV_data[2 * image_size_px + i] = Cr;
    }
    return YUV_data;
}

vector<byte_t> BMP_to_YUV420(const BMP& bmp)
{
    // TODO: bmp width is assumed to be multiple of 2, height is assumed even

    auto YUV444_data = BMP_to_YUV444(bmp);

    size_t image_size_px = bmp.width_px() * bmp.height_px();
    // 1 bpp for luminance, 1/2 bpp for chrominance
    vector<byte_t> YUV420_data(image_size_px * 3 / 2);

    // luminance remains at full resolution
    copy(begin(YUV444_data), begin(YUV444_data) + image_size_px,
         begin(YUV420_data));

    auto average = [&]() -> byte_t
    {
        static size_t pos = image_size_px;
        byte_t average = (YUV444_data[pos] +
                          YUV444_data[pos + 1] +
                          YUV444_data[pos + bmp.width_px()] +
                          YUV444_data[pos + bmp.width_px() + 1]) / 4;
        pos += 2;
        // skip every other line
        if (pos % bmp.width_px() == 0) {
            pos += bmp.width_px();
        }
        return average;
    };

    auto start_Cb = begin(YUV420_data) + image_size_px;
    auto start_Cr = begin(YUV420_data) + image_size_px + image_size_px / 4;

    generate(start_Cb, start_Cr, average);
    generate(start_Cr, end(YUV420_data), average);

    return YUV420_data;
}
