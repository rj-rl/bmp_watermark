#include <bmp.h>
#include <yuv.h>
#include <conversions.h>
#include <watermark.h>
#include <subsample_algo.h>
#include <byte.h>
#include <no_lsan_debug.h>
#include <profile.h>

#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstring>

using namespace std;
/*
    argv[1] - path to bmp
    argv[2] - path to yuv
    argv[3] - yuv width
    argv[4] - yuv height
optional:
    argv[5] - path to output {default = working directory}
    argv[6] - y coordinate of where to put the picture {default = 0}
    argv[7] - x coordinate of where to put the picture {default = 0}
*/
int main(int argc, const char* argv[])
{
    // FIXME:
    {
        LOG_DURATION("parallalel version");
        BMP t{"/home/rj_rl/Desktop/work/pics/oddity.bmp"};
        auto res = BMP_to_YUV420_simd(t);
        ofstream{"oddity_simd.yuv", ios::binary}.write(reinterpret_cast<char*>(&res.data[0]), res.data.size());
    }

    
    if (argc < 5) {
        cout << "Usage: watermark path-to-bmp path-to-yuv "
            "yuv-width yuv-height output-path [bmp_position_y bmp_position_x]\n";
        return 1;
    }

    filesystem::path path_to_bmp = argv[1];
    filesystem::path path_to_yuv = argv[2];

    size_t yuv_width  = stoul(argv[3]);
    size_t yuv_height = stoul(argv[4]);

    filesystem::path out_path = "out.yuv";
    if (argv[5]) {
        out_path = argv[5];
    }
    
    size_t pos_y = 0u;
    size_t pos_x = 0u;
    if (argc == 8) {
        pos_y = stoul(argv[6]);
        pos_x = stoul(argv[7]);
    }

    uint32_t animation_speed = 0;
    if (argc == 9) {
        animation_speed = stoi(argv[8]);
    }

    BMP bmp{path_to_bmp};
    YUV video{path_to_yuv, yuv_width, yuv_height, YUV::Type::Planar420};
    bool success = add_watermark(video, bmp, pos_y, pos_x, animation_speed);
    if (!success) {
        cerr << "Task failed successfully (pic dimensions > vid dimensions)\n";
        return 2;
    }

    ofstream out_file{out_path, ios::binary | ios::out};
    if (!out_file) {
        cerr << "Could not open file for output\n";
        return 3;
    }
    out_file.write(reinterpret_cast<char*>(&video.data[0]), video.data.size());
    return 0;
}
