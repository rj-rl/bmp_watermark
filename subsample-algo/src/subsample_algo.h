#pragma once

#include <byte.h>
#include <matrix.h>

#include <vector>
#include <algorithm>
#include <numeric>
#include <stddef.h>

// TODO: add support for different subsample sizes

namespace Subsample
{
// returns the elements of a 2x2 window (submatrix) starting at [row, col]
// window is truncated if it doesn't fit
template <typename T = Utility::byte_t>
auto select(const Utility::Matrix<const T>& mat, size_t row, size_t col)
{
    // 4 possible scenarios:
    //    1. [row, col] is not near an edge -> GREAT SUCCESS! window fits
    //    2. it's the last element in a row -> window is cut in half
    //    3. it's the last element in a column -> window cut in half
    //    4. it's the last element in the matrix -> window is this 1 element
    if (col == mat.width() - 1) {
        if (row == mat.height() - 1) {   // case 4, single value
            return std::vector{mat(row, col)};
        }
        else {                           // case 2
            return std::vector{mat(row, col), mat(row + 1, col)};
        }
    }
    else if (row == mat.height() - 1) {  // case 3
        return std::vector{mat(row, col), mat(row, col + 1)};
    }
    else                                 // case 1
        return std::vector{mat(row, col), mat(row, col + 1),
            mat(row + 1, col), mat(row + 1, col + 1)};
}

/*
    Below are the actual sampling functions.
    These were supposed to be used for RGB data in the form of vector<RGB_px>,
    but adding RGB_px's together was awkward and unintuitive.
    My day was ruined and my disappointment immeasurable.
*/

// returns arithmetic mean from a 2x2 sample at [row, col]
template <typename T = Utility::byte_t>
T mean(const Utility::Matrix<const T>& mat, size_t row, size_t col)
{
    auto subsample = select(mat, row, col);
    int64_t sum = std::accumulate(std::begin(subsample), std::end(subsample), 0);
    return sum / subsample.size();
};

// returns median value from a 2x2 sample at [row, col]
template <typename T = Utility::byte_t>
T median(const Utility::Matrix<const T>& mat, size_t row, size_t col)
{
    auto subsample = select(mat, row, col);
    if (subsample.size() == 1) return subsample[0];

    std::sort(std::begin(subsample), std::end(subsample));
    size_t middle = subsample.size() / 2;

    return middle % 2 == 0
        ? (subsample[middle - 1] + subsample[middle]) / 2
        : subsample[middle];
};

// returns max value from a 2x2 sample at [row, col]
template <typename T = Utility::byte_t>
T max(const Utility::Matrix<const T>& mat, size_t row, size_t col)
{
    auto subsample = select(mat, row, col);
    return *std::max_element(std::begin(subsample), std::end(subsample));
};

}  // ::Subsample
