#pragma once

#include <stdexcept>
#include <iterator>
#include <stddef.h>

namespace Utility {

// abstracts contiguously stored elements as a rectangular matrix
template <typename BidirIt>
class Matrix {
public:
    using T = typename BidirIt::value_type;
    
    Matrix(BidirIt begin, BidirIt end,
           size_t width = 0u, size_t height = 0u)
        : begin_{begin}
        , end_{end}
        , width_{width}
        , height_{height}
    {
        if (static_cast<size_t>(std::distance(begin, end)) != width * height) {
            throw std::runtime_error{"Invalid matrix width/height"};
        }
    }

    // returns i-th element of the matrix, counting top-to-bottom/left-to-right
    T& operator[] (size_t i)
    {
        return begin_[i];
    }

    T operator[] (size_t i) const
    {
        return begin_[i];
    }

    T& operator() (size_t row, size_t col)
    {
        return begin_[row * width_ + col];
    }

    T operator() (size_t row, size_t col) const
    {
        return begin_[row * width_ + col];
    }

    size_t width() const
    {
        return width_;
    }

    size_t height() const
    {
        return height_;
    }

    size_t size() const
    {
        return std::distance(begin_, end_);
    }

private:
    BidirIt begin_;
    BidirIt end_;
    size_t width_;
    size_t height_;
};

} // ::Utility
