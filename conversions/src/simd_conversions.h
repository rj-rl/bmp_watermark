#pragma once

#include <byte.h>

#include <emmintrin.h>
#include <tmmintrin.h>
#include <cstdint>

// partially splits stream of rgb data into separate channels
// one call doubles the clumps: rgb rgb rgb ... -> rr gg bb rr gg ...
inline void deinterleave_rgb(
    __m128i  src1, __m128i  src2, __m128i  src3, __m128i  src4, __m128i  src5, __m128i  src6,
    __m128i* dst1, __m128i* dst2, __m128i* dst3, __m128i* dst4, __m128i* dst5, __m128i* dst6)
{
    *dst1 = _mm_unpacklo_epi8(src1, src4);
    *dst2 = _mm_unpackhi_epi8(src1, src4);
    *dst3 = _mm_unpacklo_epi8(src2, src5);
    *dst4 = _mm_unpackhi_epi8(src2, src5);
    *dst5 = _mm_unpacklo_epi8(src3, src6);
    *dst6 = _mm_unpackhi_epi8(src3, src6);
}

// homebrew pair, because std::pair complains about __m128i's attributes
struct pair_128i {
    __m128i first;
    __m128i second;
};

inline pair_128i promote_8_16(__m128i src)
{
    return {_mm_unpacklo_epi8(src, _mm_setzero_si128()),
            _mm_unpackhi_epi8(src, _mm_setzero_si128())};
}

void simd_RGB_to_YUV420(const Utility::byte_t* rgb_top,
                        const Utility::byte_t* rgb_bot,
                        Utility::byte_t* dst_y_top,
                        Utility::byte_t* dst_y_bot,
                        Utility::byte_t* dst_cb,
                        Utility::byte_t* dst_cr)
{
    using Utility::byte_t;

    // the conversion coefficients
    static const auto y_r = _mm_set1_epi16(66);
    static const auto y_g = _mm_set1_epi16(129);
    static const auto y_b = _mm_set1_epi16(25);

    static const auto cb_r = _mm_set1_epi16(-38);
    static const auto cb_g = _mm_set1_epi16(-74);
    static const auto cb_b = _mm_set1_epi16(112);

    static const auto cr_r = _mm_set1_epi16(112);
    static const auto cr_g = _mm_set1_epi16(-94);
    static const auto cr_b = _mm_set1_epi16(-18);

    // vectorized 128 and 16
    static const auto const_128 = _mm_set1_epi16(128);
    static const auto const_16 = _mm_set1_epi16(16);

    // load pixel data
    __m128i p1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(rgb_top));
    __m128i p2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(rgb_top + 16));
    __m128i p3 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(rgb_top + 32));
    __m128i p4 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(rgb_bot));
    __m128i p5 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(rgb_bot + 16));
    __m128i p6 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(rgb_bot + 32));

    // separate channels
    __m128i t1, t2, t3, t4, t5, t6;  // temp variables
    deinterleave_rgb(p1, p2, p3, p4, p5, p6, &t1, &t2, &t3, &t4, &t5, &t6);  // each channel now in groups of 2
    deinterleave_rgb(t1, t2, t3, t4, t5, t6, &p1, &p2, &p3, &p4, &p5, &p6);  // groups of 4
    deinterleave_rgb(p1, p2, p3, p4, p5, p6, &t1, &t2, &t3, &t4, &t5, &t6);  // groups of 8
    deinterleave_rgb(t1, t2, t3, t4, t5, t6, &p1, &p2, &p3, &p4, &p5, &p6);  // groups of 16
    deinterleave_rgb(p1, p2, p3, p4, p5, p6, &t1, &t2, &t3, &t4, &t5, &t6);  // groups of 32

    // blue channel values for top and bottom row
    auto top_b = t1;
    auto bot_b = t2;
    // green channel
    auto top_g = t3;
    auto bot_g = t4;
    // red channel
    auto top_r = t5;
    auto bot_r = t6;

    // 8 bit isn't enough to hold intermediate values, promote to 16 bit
    auto [top_b1, top_b2] = promote_8_16(top_b);
    auto [top_g1, top_g2] = promote_8_16(top_g);
    auto [top_r1, top_r2] = promote_8_16(top_r);

    // the luma arithmetic: weigh the rgb
    auto top_y1 = _mm_add_epi16(_mm_mullo_epi16(top_b1, y_b), _mm_mullo_epi16(top_g1, y_g));
    top_y1 = _mm_add_epi16(top_y1, _mm_mullo_epi16(top_r1, y_r));
    auto top_y2 = _mm_add_epi16(_mm_mullo_epi16(top_b2, y_b), _mm_mullo_epi16(top_g2, y_g));
    top_y2 = _mm_add_epi16(top_y2, _mm_mullo_epi16(top_r2, y_r));
    // squeeze into 8 bit
    top_y1 = _mm_srli_epi16((_mm_add_epi16(top_y1, const_128)), 8);
    top_y2 = _mm_srli_epi16((_mm_add_epi16(top_y2, const_128)), 8);
    // offset
    top_y1 = _mm_add_epi16(top_y1, const_16);
    top_y2 = _mm_add_epi16(top_y2, const_16);

    // store the luma for 16 pixels, top row
    // technically breaks strict aliasing rule and is undefined behavior
    _mm_storeu_si128(reinterpret_cast<__m128i*>(dst_y_top), _mm_packus_epi16(top_y1, top_y2));
    dst_y_top += 16;

    // now the bottom row
    auto [bot_b1, bot_b2] = promote_8_16(bot_b);
    auto [bot_g1, bot_g2] = promote_8_16(bot_g);
    auto [bot_r1, bot_r2] = promote_8_16(bot_r);

    auto bot_y1 = _mm_add_epi16(_mm_mullo_epi16(bot_b1, y_b), _mm_mullo_epi16(bot_g1, y_g));
    bot_y1 = _mm_add_epi16(bot_y1, _mm_mullo_epi16(bot_r1, y_r));
    auto bot_y2 = _mm_add_epi16(_mm_mullo_epi16(bot_b2, y_b), _mm_mullo_epi16(bot_g2, y_g));
    bot_y2 = _mm_add_epi16(bot_y2, _mm_mullo_epi16(bot_r2, y_r));
    // squeeze into 8 bit
    bot_y1 = _mm_srli_epi16((_mm_add_epi16(bot_y1, const_128)), 8);
    bot_y2 = _mm_srli_epi16((_mm_add_epi16(bot_y2, const_128)), 8);
    // offset
    bot_y1 = _mm_add_epi16(bot_y1, const_16);
    bot_y2 = _mm_add_epi16(bot_y2, const_16);

    // store the luma for 16 pixels, bottom row
    _mm_storeu_si128(reinterpret_cast<__m128i*>(dst_y_bot), _mm_packus_epi16(bot_y1, bot_y2));
    dst_y_bot += 16;

    // now the chroma channels; first find the average for every 2v2 pixels
    // sum adjacent values in top row
    auto top_b_pair_sum = _mm_hadd_epi16(top_b1, top_b2);
    auto top_g_pair_sum = _mm_hadd_epi16(top_g1, top_g2);
    auto top_r_pair_sum = _mm_hadd_epi16(top_r1, top_r2);
    // sum adjacent values in bottom row
    auto bot_b_pair_sum = _mm_hadd_epi16(bot_b1, bot_b2);
    auto bot_g_pair_sum = _mm_hadd_epi16(bot_g1, bot_g2);
    auto bot_r_pair_sum = _mm_hadd_epi16(bot_r1, bot_r2);
    // add vertically to get sum of 2v2 block elements and divide by 4;
    // division can be safely done by shifting >> 2, since all values >= 0
    auto b_mean = _mm_srli_epi16(_mm_add_epi16(top_b_pair_sum, bot_b_pair_sum), 2);
    auto g_mean = _mm_srli_epi16(_mm_add_epi16(top_g_pair_sum, bot_g_pair_sum), 2);
    auto r_mean = _mm_srli_epi16(_mm_add_epi16(top_r_pair_sum, bot_r_pair_sum), 2);

    // do the transformation
    auto cb1 = _mm_add_epi16(_mm_mullo_epi16(b_mean, cb_b), _mm_mullo_epi16(g_mean, cb_g));
    cb1 = _mm_add_epi16(cb1, _mm_mullo_epi16(r_mean, cb_r));
    auto cr1 = _mm_add_epi16(_mm_mullo_epi16(b_mean, cr_b), _mm_mullo_epi16(g_mean, cr_g));
    cr1 = _mm_add_epi16(cr1, _mm_mullo_epi16(r_mean, cr_r));
    // squeeze into 8 bit and offset
    cb1 = _mm_add_epi16(_mm_srli_epi16((_mm_add_epi16(cb1, const_128)), 8), const_128);
    cr1 = _mm_add_epi16(_mm_srli_epi16((_mm_add_epi16(cr1, const_128)), 8), const_128);

    // narrow type to byte and write
    uint16_t cb[8];
    uint16_t cr[8];
    _mm_storeu_si128(reinterpret_cast<__m128i*>(cb), cb1);
    _mm_storeu_si128(reinterpret_cast<__m128i*>(cr), cr1);
    for (auto val : cb) {
        *dst_cb++ = static_cast<byte_t>(val);
    }
    for (auto val : cr) {
        *dst_cr++ = static_cast<byte_t>(val);
    }
}
