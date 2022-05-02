#pragma once

#include <vector>
#include <byte.h>

struct BMP;

std::vector<byte_t> BMP_to_YCbCr(const BMP& bmp);
