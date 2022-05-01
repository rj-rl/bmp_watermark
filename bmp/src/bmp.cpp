#include <bmp.h>

#include <string>
#include <fstream>
#include <cmath>
#include <stdexcept>

using namespace std;

BMP BMP::from_file(const string& filename)
{
    return BMP{filename};
}

BMP::BMP(const string& filename)
{
    ifstream file{filename, ios::binary | ios::in};
    if (!file) {
        throw runtime_error{"Could not open the .bmp file"};
    }

    // TODO: reverse byte order in the multibyte fields for big-endians
    file.read(reinterpret_cast<char*>(&file_header), sizeof(file_header));
    file.read(reinterpret_cast<char*>(&info_header), sizeof(info_header));

    if (file_header.file_type != 0x4D42) {
        throw runtime_error{"BMP has to have BM signature"};
    }
    if (info_header.compression != 0) {
        throw runtime_error{"Bitmap data has to be in uncompressed RGB format"};
    }
    if (info_header.bit_count != 24) {
        throw runtime_error{"Bitmap color depth has to be 24"};
    }

    size_t width = info_header.width_px * 3;
    size_t height = abs(info_header.height_px);
    size_t pixel_data_size = width * height;
    size_t pad_size = (width % 4 != 0) ? (4 - width % 4) : 0;

    // the bitmap is usually stored bottom-up
    // so we fill the pixel_data bottom-up as well
    int32_t start = height - 1;
    int32_t end = -1;
    int32_t step = -1;
    // negative height_px means the bitmap is in top-down order,
    // so we fill pixel_data top-down
    if (info_header.height_px < 0) {
        start = 0;
        end = height;
        step = 1;
    }

    pixel_data.resize(pixel_data_size);
    vector<byte_t> padding(pad_size);
    file.seekg(file_header.data_offset);
    for (int32_t i = start; i != end; i += step) {
        file.read(reinterpret_cast<char*>(&pixel_data[i * width]), width);
        if (pad_size > 0) {
            // ignore padding
            file.read(reinterpret_cast<char*>(&padding[0]), pad_size);
        }
    }
}

/*
    TV levels (aka studio swing):
        Y in [16, 235]; Cb, Cr in [16, 240]

    PC levels (aka full swing):
        Y, Cb, Cr in [0, 255]
*/

// TV levels
static vector<int32_t> conversion_matrix = {
     66, 129,  25,
    -38, -74, 112,
    112, -94, -18
};

struct YCbCr_pixel {
    byte_t Y = 0;
    byte_t Cb = 0;
    byte_t Cr = 0;
};

// converts an RGB pixel into a YCbCr pixel
YCbCr_pixel RGB_to_YCbCr_px(byte_t red, byte_t green, byte_t blue)
{
    YCbCr_pixel pixel;
    size_t row = 0u;

    // convert
    uint16_t Y = conversion_matrix[3 * row + 0] * red
        + conversion_matrix[3 * row + 1] * green
        + conversion_matrix[3 * row + 2] * blue;
    ++row;

    int16_t Cb = conversion_matrix[3 * row + 0] * red
        + conversion_matrix[3 * row + 1] * green
        + conversion_matrix[3 * row + 2] * blue;
    ++row;
    
    int16_t Cr = conversion_matrix[3 * row + 0] * red
        + conversion_matrix[3 * row + 1] * green
        + conversion_matrix[3 * row + 2] * blue;

    // scale
    Y  = (Y + 128) >> 8;
    Cb = (Y + 128) >> 8;
    Cr = (Y + 128) >> 8;

    // offset
    pixel.Y = Y + 16;
    pixel.Cb = Cb + 128;
    pixel.Cr = Cr + 128;

    return pixel;
}

vector<byte_t> BMP_to_YCbCr(const BMP& bmp)
{
    // TODO: this is 4:4:4 planar version, needs subsampling
    // TODO: bmp width is assumed to be multiple of 4, height is assumed even
    const auto& pixels = bmp.pixel_data;

    vector<byte_t> result(pixels.size());
    size_t image_size_px = bmp.info_header.width_px * bmp.info_header.height_px;

    for (size_t i = 0; i < image_size_px; ++i) {
        // BGR channel order is assumed
        auto [Y, Cb, Cr] = RGB_to_YCbCr_px(
            pixels[3 * i + 2]  /* blue */,
            pixels[3 * i + 1]  /* green */,
            pixels[3 * i]      /* red */
        );
        // luminance plane
        result[i] = Y;
        // chrominance planes
        result[image_size_px + i] = Cb;
        result[2 * image_size_px + i] = Cr;
    }
    return result;
}
