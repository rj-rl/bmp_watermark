#include <cstdint>
#include <string>
#include <vector>

#pragma pack(push, 1)  // don't pad the fields

struct BMPFileHeader {
    uint16_t file_type = 0x4D42;  // this should be 0x4D42 = 'BM'
    uint32_t file_size = 0;       // file size in bytes
    uint32_t reserved = 0;        // reserved
    uint32_t data_offset = 0;     // offset to where the actual pixel data starts
};

struct BMPInfoHeader {
    uint32_t size{ 0 };               // size of this header in bytes
    int32_t width{ 0 };               // width of bitmap in pixels
    int32_t height{ 0 };              // width of bitmap in pixels
                                        // if negative, bitmap is in top-down orientation
    uint16_t planes{ 1 };             // always 1
    uint16_t bit_count{ 0 };          // bits per pixel
    uint32_t compression{ 0 };        // 0 means uncompressed
    uint32_t size_image{ 0 };         // 0 for uncompressed images
    int32_t x_pixels_per_meter{ 0 };  // horizontal resolution (px / m)
    int32_t y_pixels_per_meter{ 0 };  // vertical resolution (px / m)
    uint32_t colors_used{ 0 };        // number of colors used
    uint32_t colors_important{ 0 };   // number of important colors used,
                                        // 0 means every color is important
};

#pragma pack(pop)

using byte_t = uint8_t;

struct BMP {
    BMPFileHeader file_header;
    BMPFileHeader info_header;
    std::vector<byte_t> pixel_data;

    static BMP from_file(const string& filename)
    {
        return BMP(filename);
    }

private:
    BMP(const string& filename);
};
