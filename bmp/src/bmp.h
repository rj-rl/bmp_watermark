#pragma once

#include <byte.h>

#include <cstdint>
#include <stddef.h>
#include <string>
#include <vector>

#pragma pack(push, 1)  // no padding please

struct BMP_File_Header {
    uint16_t file_type   = 0u;  // this should be 0x4D42 = 'BM'
    uint32_t file_size   = 0u;  // file size in bytes
    uint32_t reserved    = 0u;
    uint32_t data_offset = 0u;  // offset to where the actual pixel data starts
};

struct BMP_Info_Header {
    uint32_t size           = 0;   // size of this header in bytes
    int32_t  width          = 0;   // width of bitmap in pixels
    int32_t  height         = 0;   // width of bitmap in pixels; if value < 0,
                                     // then bitmap is in top-down orientation (!)
    uint16_t planes         = 0u;  // always 1
    uint16_t bit_count      = 0u;  // bits per pixel
    uint32_t compression    = 0u;  // 0 means uncompressed
    uint32_t size_image     = 0u;  // size of image in bytes (or 0 if uncompressed)
    int32_t  x_px_per_meter = 0;   // horizontal resolution (px / m)
    int32_t  y_px_per_meter = 0;   // vertical resolution (px / m)
    uint32_t clrs_used      = 0u;  // number of colors used
    uint32_t clrs_imprtont  = 0u;  // number of important colors used,
};

// BGR channel order is assumed
struct RGB_px {
    Utility::byte_t B = 0u;
    Utility::byte_t G = 0u;
    Utility::byte_t R = 0u;
};

#pragma pack(pop)

struct BMP {
    BMP_File_Header     file_header;
    BMP_Info_Header     info_header;
    std::vector<RGB_px> data;

    explicit BMP(const std::string& filename);
    size_t   width() const;
    size_t   height() const;
};
