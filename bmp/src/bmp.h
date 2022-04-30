#include <cstdint>
#include <string>
#include <vector>

#pragma pack(push, 1)  // don't pad the fields

struct BMPFileHeader {
    uint16_t file_type   = 0x4D42;  // this should be 0x4D42 = 'BM'
    uint32_t file_size   = 0;       // file size in bytes
    uint32_t reserved    = 0;       // reserved
    uint32_t data_offset = 0;       // offset to where the actual pixel data starts
};

struct BMPInfoHeader {
    uint32_t size           = 0;  // size of this header in bytes
    int32_t  width_px       = 0;  // width of bitmap in pixels
    int32_t  height_px      = 0;  // width of bitmap in pixels; negative value
                                    // means bitmap is in top-down orientation (!)
    uint16_t planes         = 1;  // always 1
    uint16_t bit_count      = 0;  // bits per pixel
    uint32_t compression    = 0;  // 0 means uncompressed
    uint32_t size_image     = 0;  // size of image in bytes (or 0 if uncompressed)
    int32_t  x_px_per_meter = 0;  // horizontal resolution (px / m)
    int32_t  y_px_per_meter = 0;  // vertical resolution (px / m)
    uint32_t clrs_used      = 0;  // number of colors used
    uint32_t clrs_imprtont  = 0;  // number of important colors used,
};

#pragma pack(pop)

using byte_t = uint8_t;

struct BMP {
    BMPFileHeader       file_header;
    BMPInfoHeader       info_header;
    // TODO: do we really need these 2?
    std::size_t         width;         // in bytes
    std::size_t         padded_width;  // in bytes
    std::vector<byte_t> pixel_data;

    static BMP from_file(const std::string& filename);

private:
    explicit BMP(const std::string& filename);
};
