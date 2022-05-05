#include <watermark.h>
#include <conversions.h>
#include <matrix.h>

#include <algorithm>
#include <cassert>

using namespace std;

bool add_watermark(YUV& vid, const BMP& bmp,
                   int32_t pos_y, int32_t pos_x, int32_t speed)
{
    if (bmp.height() > vid.height || bmp.width() > vid.width) {
        return false;
    }
    // make sure the picture fits on the canvas
    pos_y = min<size_t>(pos_y, vid.height - bmp.height());
    pos_x = min<size_t>(pos_x, vid.width - bmp.width());

    const auto pic = BMP_to_YUV420(bmp);
    // total number of BOTH chroma components per image/frame
    const auto src_chroma_size = chroma_count_420(pic.width, pic.height);
    const auto dst_chroma_size = chroma_count_420(vid.width, vid.height);

    auto frame_begin = vid.data.begin();
    // sizes of frame in pixels and bytes
    const auto frame_size = vid.width * vid.height;
    const auto bytes_per_frame = frame_size + dst_chroma_size;
    // width of chroma planes
    const auto src_chroma_width = (pic.width + 1) / 2;
    const auto dst_chroma_width = (vid.width + 1) / 2;

    // offsets from beginning of chroma data, i.e. beginning of pic's *data*
    const auto src_Cb_offset = pic.width * pic.height;
    const auto src_Cr_offset = src_Cb_offset + src_chroma_size / 2;

    int32_t step_y = 2 * speed;  // 2, because chroma isn't sampled
    int32_t step_x = 2 * speed;  // at odd columns and rows

    for (size_t frame_num = 0u; frame_num < vid.frame_count(); ++frame_num) {
        // offsets from beginning of chroma data, i.e. beginning of current *frame*
        const auto dst_Cb_offset = frame_size + (pos_y / 2 * vid.width + pos_x) / 2;
        const auto dst_Cr_offset = dst_Cb_offset + dst_chroma_size / 2;

        auto src_Y = begin(pic.data);
        auto src_Cb = src_Y + src_Cb_offset;
        auto src_Cr = src_Y + src_Cr_offset;

        auto dst_Y = frame_begin + pos_y * vid.width + pos_x;
        auto dst_Cb = frame_begin + dst_Cb_offset;
        auto dst_Cr = frame_begin + dst_Cr_offset;

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
        if (pos_y + pic.height + step_y > vid.height
            || pos_y + step_y < 0) {
            step_y *= (-1);
        }
        if (pos_x + pic.width + step_x > vid.width
            || pos_x + step_x < 0) {
            step_x *= (-1);
        }
        pos_y += step_y;
        pos_x += step_x;
        // NEXT FRAME PLEASE!
        frame_begin += bytes_per_frame;
    }
    return true;
}
