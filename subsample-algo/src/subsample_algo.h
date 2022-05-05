#pragma once

#include <byte.h>
#include <matrix.h>

#include <vector>
#include <algorithm>
#include <execution>
#include <numeric>
#include <stddef.h>

// TODO: add support for different subsample sizes

namespace Sample
{
// returns the elements of a 2x2 window (submatrix) starting at [row, col]
// window is truncated if it doesn't fit
template <typename T>
std::vector<T> select(const Utility::Matrix<const T>& mat,
                      size_t row, size_t col)
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

// returns sample value from a 2v2 window in matrix
// calculated according to a given sampling algorithm
template<typename TValue, typename TSampler>
TValue sample(const Utility::Matrix<const TValue>& mat,
              size_t row, size_t col, TSampler sampler)
{
    auto elems = select(mat, row, col);
    return sampler(elems);
}

/*
    Below are the actual sampling functions.
    These were supposed to be used for RGB data in the form of vector<RGB_px>,
    but adding RGB_px's together was awkward and unintuitive.
    My day was ruined and my disappointment immeasurable.
    I ended up using custom mean for RGB_px
*/

// returns arithmetic mean of vector elements
template <typename T = Utility::byte_t>
T mean(const std::vector<T>& elems)
{
    // note: std::reduce was a lot slower
    int64_t sum = std::accumulate(std::begin(elems), std::end(elems), 0);
    return sum / elems.size();
};

// returns median value from a 2x2 sample at [row, col]
template <typename T = Utility::byte_t>
T median(std::vector<T>& elems)
{
    if (elems.size() == 1) return elems[0];

    std::sort(std::begin(elems), std::end(elems));
    size_t middle = elems.size() / 2;

    return middle % 2 == 0
        ? (elems[middle - 1] + elems[middle]) / 2
        : elems[middle];
};

// returns max value from a 2x2 sample at [row, col]
template <typename T = Utility::byte_t>
T max(const std::vector<T>& elems)
{
    return *std::max_element(std::begin(elems), std::end(elems));
};

}  // ::Sample
