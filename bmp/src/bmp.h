#pragma once

#include <byte.h>

#include <cstdint>
#include <stddef.h>
#include <string>
#include <vector>

#pragma pack(push, 1)  // don't pad the fields

struct BMP_File_Header {
    uint16_t file_type;       // this should be 0x4D42 = 'BM'
    uint32_t file_size;       // file size in bytes
    uint32_t reserved;
    uint32_t data_offset;     // offset to where the actual pixel data starts
};

struct BMP_Info_Header {
    uint32_t size;            // size of this header in bytes
    int32_t  width;        // width of bitmap in pixels
    int32_t  height;       // width of bitmap in pixels; if value < 0,
                                 // then bitmap is in top-down orientation (!)
    uint16_t planes;          // always 1
    uint16_t bit_count;       // bits per pixel
    uint32_t compression;     // 0 means uncompressed
    uint32_t size_image;      // size of image in bytes (or 0 if uncompressed)
    int32_t  x_px_per_meter;  // horizontal resolution (px / m)
    int32_t  y_px_per_meter;  // vertical resolution (px / m)
    uint32_t clrs_used;       // number of colors used
    uint32_t clrs_imprtont;   // number of important colors used,
};

#pragma pack(pop)

// BGR channel order is assumed
struct RGB_px {
    Utility::byte_t B = 0u;
    Utility::byte_t G = 0u;
    Utility::byte_t R = 0u;
};

struct BMP {
    BMP_File_Header     file_header;
    BMP_Info_Header     info_header;
    std::vector<RGB_px> pixel_data;

    static BMP from_file(const std::string& filename);
    size_t width() const;
    size_t height() const;

private:
    explicit BMP(const std::string& filename);
};
