#include <watermark.h>
#include <conversions.h>
#include <matrix.h>

#include <algorithm>
#include <cassert>

using namespace std;

bool add_watermark(YUV& vid, const BMP& bmp)
{
    if (bmp.height() > vid.height || bmp.width() > vid.width) {
        return false;
    }

    const auto pic = BMP_to_YUV420(bmp);
    // total number of BOTH chroma components per image/frame
    const auto src_chroma_size = chroma_count_420(pic.width, pic.height);
    const auto dst_chroma_size = chroma_count_420(vid.width, vid.height);

    auto frame_begin = vid.data.begin();
    // sizes of frame in pixels and bytes
    const auto frame_size = vid.width * vid.height;
    const auto frame_size_B = frame_size + dst_chroma_size;
    // width of chroma planes
    const auto src_chroma_width = (pic.width + 1) / 2;
    const auto dst_chroma_width = (vid.width + 1) / 2;

    const auto src_Cb_offset = pic.width * pic.height;
    const auto src_Cr_offset = src_Cb_offset + src_chroma_size / 2;

    const auto dst_Cb_offset = frame_size;
    const auto dst_Cr_offset = dst_Cb_offset + dst_chroma_size / 2;

    for (size_t frame_num = 0u; frame_num < vid.frame_count(); ++frame_num) {
        auto src_Y = begin(pic.data);
        auto src_Cb = src_Y + src_Cb_offset;
        auto src_Cr = src_Y + src_Cr_offset;

        auto dst_Y = frame_begin;
        auto dst_Cb = dst_Y + dst_Cb_offset;
        auto dst_Cr = dst_Y + dst_Cr_offset;

        for (size_t row = 0u; row < pic.height; ++row) {
            // write one row worth of luma data
            copy(src_Y, src_Y + pic.width, dst_Y);
            // move both ptrs to their next respective row
            src_Y += pic.width;
            dst_Y += vid.width;
            // chroma is sampled every other row
            if (row % 2 == 0) {
                // write one row of each of the chroma components
                copy(src_Cb, src_Cb + src_chroma_width, dst_Cb);
                copy(src_Cr, src_Cr + src_chroma_width, dst_Cr);
                // move all ptrs to their next respective row
                src_Cb += src_chroma_width;
                src_Cr += src_chroma_width;
                dst_Cb += dst_chroma_width;
                dst_Cr += dst_chroma_width;
            }
        }
        // NEXT FRAME PLEASE!
        frame_begin += frame_size_B;
    }
    return true;
}
