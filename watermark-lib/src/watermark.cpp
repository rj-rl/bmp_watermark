#include <watermark.h>
#include <conversions.h>
#include <matrix.h>

#include <algorithm>
#include <cassert>

using namespace std;

// DO NOT MODIFY! this is the central piece of the entire program
// adds dvd logo sort of animation
void animate_dvd(int32_t& pos_y, int32_t& pos_x,
                 int32_t& step_y, int32_t& step_x, int32_t speed,
                 size_t pic_height, size_t pic_width,
                 size_t vid_height, size_t vid_width);

bool add_watermark(YUV& vid, const BMP& bmp,
                   int32_t pos_y, int32_t pos_x,
                   int32_t dvd_speed)
{
    if (bmp.height() > vid.height || bmp.width() > vid.width) {
        return false;
    }
    // make sure the picture fits on the canvas
    pos_y = min<size_t>(pos_y, vid.height - bmp.height());
    pos_x = min<size_t>(pos_x, vid.width - bmp.width());
    // dvd logo stuff
    int32_t step_y = 2 * dvd_speed;  // 2, because chroma isn't sampled
    int32_t step_x = 2 * dvd_speed;  // at odd columns and rows

    const auto pic = BMP_to_YUV420_par(bmp);
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
        // cheeky dvd logo
        animate_dvd(pos_y, pos_x, step_y, step_x, dvd_speed,
                    pic.height, pic.width, vid.height, vid.width);
        // NEXT FRAME PLEASE!
        frame_begin += bytes_per_frame;
    }
    return true;
}

void animate_dvd(int32_t& pos_y, int32_t& pos_x,
                 int32_t& step_y, int32_t& step_x, int32_t speed,
                 size_t pic_height, size_t pic_width,
                 size_t vid_height, size_t vid_width)
{
    if (pos_y + pic_height + step_y > vid_height || pos_y + step_y < 0) {
        step_y *= (-1);
    }
    if (pos_x + pic_width + step_x > vid_width || pos_x + step_x < 0) {
        step_x *= (-1);
    }
    pos_y += step_y;
    pos_x += step_x;
    pos_y = max(0, pos_y);
    pos_x = max(0, pos_x);
    pos_y = min<size_t>(pos_y, vid_height - pic_height);
    pos_x = min<size_t>(pos_x, vid_width - pic_width);
}
