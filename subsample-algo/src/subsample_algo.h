#pragma once

#include <byte.h>

#include <vector>
#include <cstddef>

namespace Subsample
{

byte_t mean(const std::vector<byte_t>& data,
            std::size_t width_px, std::size_t height_px,
            std::size_t pos);

byte_t median(const std::vector<byte_t>& data,
              std::size_t width_px, std::size_t height_px,
              std::size_t pos);

byte_t maximum(const std::vector<byte_t>& data,
               std::size_t width_px, std::size_t height_px,
               std::size_t pos);

}  // ::Subsample
