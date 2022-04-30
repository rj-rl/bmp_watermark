#include <bmp.h>
#include <string>
#include <fstream>
#include <stdexcept>

using namespace std;

BMP BMP::from_file(const std::string& filename)
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

    if (info_header.compression != 0) {
        throw runtime_error{"Bitmap data must be in uncompressed RGB format"};
    }
    if (info_header.bit_count != 24) {
        throw runtime_error{"Bitmap color depth must be 24"};
    }

    width = info_header.width_px * 3;
    size_t padding = (width % 4 != 0) ? (4 - width % 4) : 0;
    padded_width = width + padding;
    size_t pixel_data_size = padded_width * info_header.height_px;
    
    pixel_data.resize(pixel_data_size);
    file.seekg(file_header.data_offset);
    file.read(reinterpret_cast<char*>(pixel_data.data()), pixel_data_size);
}
