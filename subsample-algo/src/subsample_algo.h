#pragma once

#include <byte.h>
#include <matrix.h>

#include <vector>
#include <algorithm>
#include <numeric>
#include <stddef.h>

using std::vector;

namespace Subsample
{

// returns the elements of a 2x2 submatrix starting at [row, col]
// submatrix is truncated if it doesn't fit
template <typename BidirIt>
auto select(const Utility::Matrix<BidirIt>& mat, size_t row, size_t col)
{
    // 4 cases:
    //    1. [row, col] is not near an edge -> GREAT SUCCESS! submatrix fits
    //    2. it's the last element in a row -> submatrix is cut in half
    //    3. it's the last element in a column -> submatrix cut in half
    //    4. it's the last element in the matrix -> submatrix is this 1 element
    if (col == mat.width() - 1) {
        if (row == mat.height() - 1) {  // case 4, single value
            return vector{mat(row, col)};
        }
        else {  // case 2
            return vector{mat(row, col), mat(row + 1, col)};
        }
    }
    else if (row == mat.height() - 1) {  // case 3
        return vector{mat(row, col), mat(row, col + 1)};
    }
    else  // case 1
        return vector{mat(row, col), mat(row, col + 1),
            mat(row + 1, col), mat(row + 1, col + 1)};
}

template <typename BidirIt>
Utility::byte_t mean_mat(const Utility::Matrix<BidirIt>& mat, size_t row, size_t col)
{
    auto subsample = select(mat, row, col);
    size_t sum = 0u;
    sum = std::accumulate(std::begin(subsample), std::end(subsample), 0u);
    return sum / subsample.size();
};

Utility::byte_t mean(const std::vector<Utility::byte_t>& data,
                     size_t width, size_t height,
                     size_t x, size_t y);

// Utility::byte_t median(const std::vector<Utility::byte_t>& data,
//                        size_t width, size_t height,
//                        size_t pos);

// Utility::byte_t maximum(const std::vector<Utility::byte_t>& data,
//                         size_t width, size_t height,
//                         size_t pos);

}  // ::Subsample
