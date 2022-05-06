// TODO: parse command line arguments

#include <bmp.h>
#include <yuv.h>
#include <conversions.h>
#include <watermark.h>
#include <subsample_algo.h>
#include <byte.h>
#include <no_lsan_debug.h>
#include <profile.h>

#include <iostream> // temp
#include <fstream>  // temp
#include <string>
#include <filesystem>
#include <vector>

using namespace std;
using namespace Utility;

/*
    argv[1] - path to bmp
    argv[2] - path to yuv
    argv[3] - yuv width
    argv[4] - yuv height
*/
int main(int argc, const char* argv[])
{
    //YUV video{"/home/rj_rl/Desktop/work/pics/flower.yuv", 352, 288, YUV::Type::Planar420};
    BMP bmp{"/home/rj_rl/Desktop/work/pics/huge.bmp"};
    {
        LOG_DURATION("Sequential version");
        auto res = BMP_to_YUV420(bmp);
        res = BMP_to_YUV420(bmp);
        res = BMP_to_YUV420(bmp);
        res = BMP_to_YUV420(bmp);
        cout << res.data[1000] << ' ';
    }
    {
        LOG_DURATION("Parallel version");
        auto res_multi = BMP_to_YUV420_par(bmp);
        res_multi = BMP_to_YUV420_par(bmp);
        res_multi = BMP_to_YUV420_par(bmp);
        res_multi = BMP_to_YUV420_par(bmp);
        cout << res_multi.data[1000] << '\n';
    }

    //add_watermark(video, bmp);

    // ofstream{"/home/rj_rl/Desktop/work/output-mine/huge-flower-MULTI.yuv", ios::binary}.write(
    //     reinterpret_cast<char*>(&video.data[0]), video.data.size()
    // );

    return 0;
}
