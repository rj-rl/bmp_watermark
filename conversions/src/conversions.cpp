#include <conversions.h>
#include <subsample_algo.h>
#include <bmp.h>
#include <yuv.h>

#include <algorithm>
#include <cmath>
#include <future>
#include <thread>
#include <cassert>

using namespace std;
using namespace Utility;

namespace {

/*
    Two versions of RGB->YCbCr transformation matrices:

    TV levels (aka studio swing):
        normalizes output so that Y is in [16, 235] and Cb, Cr in [16, 240]

    PC levels (aka full swing):
        does not limit output range; Y, Cb, Cr in [0, 255]
*/
constexpr int32_t TV_matrix[] = {
     66,  129,   25,
    -38,  -74,  112,
    112,  -94,  -18
};

constexpr int32_t PC_matrix[] = {
     77,  150,   29,
    -43,  -84,  127,
    127, -106,  -21
};

// =================== various utility functions & types ===================== //

// returns arithmetically mean pixel among a group of pixels
RGB_px mean_RGB_px(const vector<RGB_px>& pixels)
{
    int32_t R = 0, G = 0, B = 0;
    for (auto px : pixels) {
        R += px.R;
        G += px.G;
        B += px.B;
    }
    return RGB_px{
        .B = static_cast<byte_t>(B / pixels.size()),
        .G = static_cast<byte_t>(G / pixels.size()),
        .R = static_cast<byte_t>(R / pixels.size())
    };
}

// converts an RGB pixel into the luma component of a YCbCr pixel
byte_t RGB_to_Y(RGB_px rgb_px)
{
    uint16_t Y = TV_matrix[0] * rgb_px.R
        + TV_matrix[1] * rgb_px.G
        + TV_matrix[2] * rgb_px.B;
    // scale down to 8 bit
    Y = (Y + 128) >> 8;
    // offset
    return Y + 16;
}

struct Chroma {
    byte_t Cb = 0u;
    byte_t Cr = 0u;
};

// converts an RGB pixel into the two chroma components of a YCbCr pixel
Chroma RGB_to_CbCr(RGB_px rgb_px)
{
    int16_t Cb = TV_matrix[3] * rgb_px.R
        + TV_matrix[4] * rgb_px.G
        + TV_matrix[5] * rgb_px.B;

    int16_t Cr = TV_matrix[6] * rgb_px.R
        + TV_matrix[7] * rgb_px.G
        + TV_matrix[8] * rgb_px.B;
    // scale down to 8 bit
    Cb = (Cb + 128) >> 8;
    Cr = (Cr + 128) >> 8;
    // offset
    Cb = Cb + 128;
    Cr = Cr + 128;
    // sanity check
    assert(Cb > 0 && Cr > 0);
    return Chroma(Cb, Cr);
}

// converts an RGB pixel into a YCbCr pixel
YCbCr_px RGB_to_YCbCr_px(RGB_px rgb_px)
{
    auto Y = RGB_to_Y(rgb_px);
    auto [Cb, Cr] = RGB_to_CbCr(rgb_px);
    return YCbCr_px{Y, Cb, Cr};
}

// vertically divides a matrix into submatrices
template <typename T>
vector<Matrix<T>>
split_into_batches(const Matrix<T>& matrix, size_t batch_height)
{
    vector<Matrix<T>> batches;
    auto batch_begin = begin(matrix);
    size_t row = 0u;

    while (row < matrix.height()) {
        // check if the batch fits, truncate if needed
        auto current_batch_height = min(batch_height, matrix.height() - row);
        batches.emplace_back(batch_begin, matrix.width(), current_batch_height);

        row += current_batch_height;
        batch_begin += current_batch_height * matrix.width();
    }
    return batches;
}

void process_batch(Matrix<const RGB_px> RGB_mat,
                   vector<byte_t>::iterator dst_Y,
                   vector<byte_t>::iterator dst_Cb,
                   vector<byte_t>::iterator dst_Cr)
{
    for (size_t row = 0u; row < RGB_mat.height(); ++row) {
        for (size_t col = 0u; col < RGB_mat.width(); ++col) {
            // fill luma plane
            *dst_Y++ = RGB_to_Y(RGB_mat(row, col));

            // sample every other pixel in every other row
            if (row % 2 == 0 && col % 2 == 0) {
                // calculate mean RGB values
                auto [Cb, Cr] = RGB_to_CbCr(
                    Sample::sample(RGB_mat, row, col, mean_RGB_px)
                );
                // fill chrominance planes
                *dst_Cb++ = Cb;
                *dst_Cr++ = Cr;
            }
        }
    }
}

}  // ::unnamed

// ========================= conversion functions ========================== //

YUV BMP_to_YUV444(const BMP& bmp)
{
    const auto width = bmp.width();
    const auto height = bmp.height();
    const auto image_size_px = width * height;

    vector<byte_t> yuv_data(image_size_px * 3);  // 3 bytes per pixel
    const auto& RGB_data = bmp.data;

    for (size_t i = 0; i < image_size_px; ++i) {
        auto [Y, Cb, Cr] = RGB_to_YCbCr_px(RGB_data[i]);
        // luminance plane
        yuv_data[i] = Y;
        // chrominance planes
        yuv_data[image_size_px + i] = Cb;
        yuv_data[2 * image_size_px + i] = Cr;
    }
    return YUV{move(yuv_data), width, height, YUV::Type::Planar444};
}

YUV BMP_to_YUV420(const BMP& bmp)
{
    const auto width = bmp.width();
    const auto height = bmp.height();
    const auto image_size_px = width * height;
    // number of chroma subsamples
    const auto chroma_sub_count = chroma_count_420(width, height);

    // set up the output
    vector<byte_t> yuv_data(image_size_px + chroma_sub_count);
    auto dst_Y = begin(yuv_data);
    auto dst_Cb = dst_Y + image_size_px;
    auto dst_Cr = dst_Cb + chroma_sub_count / 2;

    const Matrix<const RGB_px> RGB_mat{begin(bmp.data), width, height};

    for (size_t row = 0u; row < height; ++row) {
        for (size_t col = 0u; col < width; ++col) {
            // fill luma plane
            *dst_Y++ = RGB_to_Y(RGB_mat(row, col));

            // sample every other pixel in every other row
            if (row % 2 == 0 && col % 2 == 0) {
                // calculate mean RGB values
                auto [Cb, Cr] = RGB_to_CbCr(
                    Sample::sample(RGB_mat, row, col, mean_RGB_px)
                );
                // fill chrominance planes
                *dst_Cb++ = Cb;
                *dst_Cr++ = Cr;
            }
        }
    }
    return YUV{move(yuv_data), width, height, YUV::Type::Planar420};
}

YUV BMP_to_YUV420_par(const BMP& bmp)
{
    const auto width = bmp.width();
    const auto height = bmp.height();
    const auto image_size_px = width * height;
    // number of chroma subsamples
    const auto chroma_sub_count = chroma_count_420(width, height);

    // set up the output
    vector<byte_t> yuv_data(image_size_px + chroma_sub_count);
    auto dst_Y = begin(yuv_data);
    auto dst_Cb = dst_Y + image_size_px;
    auto dst_Cr = dst_Cb + chroma_sub_count / 2;

    const size_t concurrency = max(4u, thread::hardware_concurrency());
    auto batch_height =
        max<size_t>(2, (height + (concurrency - 1)) / concurrency);
    // we want batches to be of even height because of the subsampling
    if (batch_height % 2 == 1) {
        ++batch_height;
    }

    const auto batches = split_into_batches(
        Matrix<const RGB_px>{begin(bmp.data), width, height},
        batch_height
    );
    vector<future<void>> futures;

    for (auto batch : batches) {
        futures.push_back(async(process_batch, batch, dst_Y, dst_Cb, dst_Cr));
        // shift the destination pointers by how much we've just filled
        // the respective planes: Y by the number of elements in batch
        // Cb and Cr by 
        dst_Y += batch.size();
        dst_Cb += chroma_count_420(width, batch.height())/2;
        dst_Cr += chroma_count_420(width, batch.height())/2;
    }
    for (auto& future : futures) {
        future.get();
    }
    return YUV{move(yuv_data), width, height, YUV::Type::Planar420};
}
