#pragma once

#include <vector>
#include <iterator>
#include <stdexcept>
#include <concepts>
#include <stddef.h>

// TODO: add stride support

namespace Utility {

// abstracts contiguously stored elements as a rectangular matrix
template <typename TValue = const byte_t>
class Matrix {
public:
    template <typename TContiguous_Iter>
        requires std::contiguous_iterator<TContiguous_Iter>
    Matrix(TContiguous_Iter begin, size_t width = 0u, size_t height = 0u)
        : begin_{&*begin}  // convert iterator to pointer
        , width_{width}
        , height_{height}
    {
        static_assert(std::same_as<
            typename std::iterator_traits<TContiguous_Iter>::value_type,
            std::remove_const_t<TValue> >
        );
    }

    // returns i-th element of the matrix, counting top-to-bottom/left-to-right
    TValue& operator[] (size_t i) 
    {
        return begin_[i];
    }
    TValue operator[] (size_t i) const
    {
        return begin_[i];
    }
    // returns element [row, col] as if by indexing two dimensional array
    TValue& operator() (size_t row, size_t col)
    {
        return begin_[row * width_ + col];
    }
    TValue operator() (size_t row, size_t col) const
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
        return width_ * height_;
    }

    auto begin() const
    {
        return begin_;
    }
    auto end() const
    {
        return begin_ + size();
    }

private:
    TValue* begin_;
    size_t width_;
    size_t height_;
};

}  // end of ::Utility
