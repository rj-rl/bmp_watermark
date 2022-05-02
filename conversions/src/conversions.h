#pragma once

#include <vector>
#include <byte.h>

struct BMP;

std::vector<byte_t> BMP_to_YUV444(const BMP& bmp);

std::vector<byte_t> BMP_to_YUV420(const BMP& bmp);
