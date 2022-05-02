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
