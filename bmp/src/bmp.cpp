#include <bmp.h>

#include <fstream>
#include <cmath>
#include <stdexcept>

using namespace std;
using namespace Utility;

byte_t byte_clamp(int32_t num)
{
    if (num < 0) return 0u;
    if (num > 255) return 255;
    return num;
}

bool   operator< (const RGB_px& a, const RGB_px& b)
{
    return (a.R + a.G + a.B) < (b.R + b.G + b.B);
}

RGB_px operator+ (const RGB_px& a, const RGB_px& b)
{
    return RGB_px{
        .B = byte_clamp(a.B + b.B),
        .G = byte_clamp(a.G + b.G),
        .R = byte_clamp(a.R + b.R)
    };
}

RGB_px operator/ (const RGB_px& a, size_t num)
{
    return RGB_px{
        .B = static_cast<byte_t>(a.B / num),
        .G = static_cast<byte_t>(a.G / num),
        .R = static_cast<byte_t>(a.R / num)
    };
}

BMP BMP::from_file(const string& filename)
{
    return BMP{filename};
}

size_t BMP::width() const
{
    return info_header.width;
}

size_t BMP::height() const
{
    return -info_header.height;
}


BMP::BMP(const string& filename)
{
    ifstream file{filename, ios::binary | ios::in};
    if (!file) {
        throw runtime_error{"Could not open the BMP file"};
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

    size_t width_B = info_header.width * 3;  // width in bytes
    size_t height = abs(info_header.height);

    // the bitmap is usually stored bottom-up;
    // we flip it by filling the pixel_data in reverse order
    int32_t start = height - 1;
    int32_t end = -1;
    int32_t step = -1;
    // negative height means the bitmap is in top-down order,
    // so we fill pixel_data in natural order
    if (info_header.height < 0) {
        start = 0;
        end = height;
        step = 1;
    }

    size_t pixel_count = info_header.width * height;
    pixel_data.resize(pixel_count);

    size_t pad_size = (width_B % 4 != 0) ? (4 - width_B % 4) : 0;
    vector<byte_t> padding(pad_size);

    file.seekg(file_header.data_offset);
    for (int32_t i = start; i != end; i += step) {
        file.read(reinterpret_cast<char*>(&pixel_data[i * info_header.width]), width_B);
        if (pad_size > 0) {
            // ignore padding
            file.read(reinterpret_cast<char*>(&padding[0]), pad_size);
        }
    }
    // bitmap is now stored top-down, update the height field
    info_header.height = -abs(info_header.height);
}
