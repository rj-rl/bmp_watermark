#include <subsample_algo.h>

#include <vector>
#include <algorithm>
#include <numeric>

using namespace std;
using namespace Utility;

// TODO: add support for different subsample sizes

// a few subsampling methods
namespace Subsample
{

byte_t mean(const vector<byte_t>& data,
            size_t width, size_t height,
            size_t x, size_t y)
{
    size_t pos = y * width + x;
    size_t pos_next_row = (y + 1) * width + x;
    // 4 cases:
    //    1. everything is even -> GREAT SUCCESS! subsample all 4 values
    //    2. width is odd -> rightmost subsample in every row is cut in half
    //    3. height is odd -> each subsample along the bottom edge is cut in half
    //    4. both odd -> both of the above + bottom-right subsample is a single value
    if ((x + 1) % width == 0) {
        if ((y + 1) % height == 0) {  // case 4, single value
            return data[pos];
        }
        else {  // case 2
            return (data[pos] + data[pos_next_row]) / 2;
        }
    }
    else if ((y + 1) % height == 0) {  // case 3
        return (data[pos] + data[pos + 1]) / 2;
    }
    else  // case 1
        return (data[pos] + data[pos + 1] +
            data[pos_next_row] + data[pos_next_row + 1]) / 4;
};



// byte_t median(const vector<byte_t>& data,
//               size_t width, size_t height,
//               size_t pos)
// {
//     // cheeky mid values calculation
//     auto mid_1 = max(min(data[pos], data[pos + 1]),
//                      min(data[pos + width], data[pos + width + 1]));
//     auto mid_2 = min(max(data[pos], data[pos + 1]),
//                      max(data[pos + width], data[pos + width + 1]));
//     return (mid_1 + mid_2) / 2;
// }

// byte_t maximum(const vector<byte_t>& data,
//                size_t width, size_t height,
//                size_t pos)
// {
//     return max(max(data[pos], data[pos + 1]),
//                max(data[pos + width], data[pos + width + 1]));
// }

}
