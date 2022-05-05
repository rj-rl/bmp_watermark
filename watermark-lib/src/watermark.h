#pragma once

#include <bmp.h>
#include <yuv.h>

#include <cstdint>

// puts a bmp on top of yuv, returns true if successful
bool add_watermark(YUV& yuv, const BMP& bmp,
                   int32_t pos_y = 50, int32_t pos_x = 0, int32_t speed = 2);
