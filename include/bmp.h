#include <cstdint>

#pragma pack(push, 1)  // don't pad the fields

struct BMPFileHeader {
    uint16_t file_type = 0x4D42;  // this should be 0x4D42 = 'BM'
    uint32_t file_size = 0;       // file size in bytes
    uint16_t reserved1 = 0;       // reserved
    uint16_t reserved2 = 0;       // reserved
    uint32_t data_offset = 0;     // offset to where the actual pixel data starts
};

struct BMPInfoHeader {
    uint32_t size{ 0 };                // Size of this header (in bytes)
    int32_t width{ 0 };                // width of bitmap in pixels
    int32_t height{ 0 };               // width of bitmap in pixels
                                            // (if positive, bottom-up, with origin in lower left corner)
                                            // (if negative, top-down, with origin in upper left corner)
    uint16_t planes{ 1 };              // always 1
    uint16_t bit_count{ 0 };           // bits per pixel
    uint32_t compression{ 0 };         // 0 means uncompressed
    uint32_t size_image{ 0 };          // 0 for uncompressed images
    int32_t x_pixels_per_meter{ 0 };
    int32_t y_pixels_per_meter{ 0 };
    uint32_t colors_used{ 0 };         // No. color indexes in the color table. Use 0 for the max number of colors allowed by bit_count
    uint32_t colors_important{ 0 };    // No. of colors used for displaying the bitmap. If 0 all colors are required
    
};

#pragma pack(pop)
