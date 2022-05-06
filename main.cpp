// TODO: parse command line arguments

#include <bmp.h>
#include <yuv.h>
#include <conversions.h>
#include <watermark.h>
#include <subsample_algo.h>
#include <byte.h>
#include <no_lsan_debug.h>

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
    YUV video{"/home/rj_rl/Desktop/work/pics/flower.yuv", 352, 288, YUV::Type::Planar420};
    BMP bmp{"/home/rj_rl/Desktop/work/pics/crourb.bmp"};

    add_watermark(video, bmp);

    ofstream{"/home/rj_rl/Desktop/work/output-mine/crourb-flower.yuv", ios::binary}.write(
        reinterpret_cast<char*>(&video.data[0]), video.data.size()
    );

    return 0;
}
