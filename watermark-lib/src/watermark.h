#pragma once

#include <bmp.h>
#include <yuv.h>

// puts a bmp on top of yuv, returns true if successful
bool add_watermark(YUV& yuv, const BMP& bmp);
