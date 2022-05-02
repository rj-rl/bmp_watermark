#include <subsample_algo.h>

#include <algorithm>

using namespace std;

namespace Subsample 
{

byte_t mean(const vector<byte_t>& data,
            size_t width_px, size_t height_px,
            size_t pos)
{
    byte_t mean = (data[pos] + data[pos + 1] +
                   data[pos + width_px] + data[pos + width_px + 1]) / 4;
    return mean;
};

byte_t median(const vector<byte_t>& data,
              size_t width_px, size_t height_px,
              size_t pos)
{
    // cheeky mid values calculation
    auto mid_1 = max(min(data[pos], data[pos + 1]),
                     min(data[pos + width_px], data[pos + width_px + 1]));
    auto mid_2 = min(max(data[pos], data[pos + 1]),
                     max(data[pos + width_px], data[pos + width_px + 1]));
    return (mid_1 + mid_2) / 2;
}

byte_t maximum(const vector<byte_t>& data,
               size_t width_px, size_t height_px,
               size_t pos)
{
    return max(max(data[pos], data[pos + 1]),
               max(data[pos + width_px], data[pos + width_px + 1]));
}

}
