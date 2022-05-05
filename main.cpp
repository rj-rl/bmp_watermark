// TODO: Размер bmp картинки может быть меньше либо равен размеру картинки входного видеоряда
// TODO: parse command line arguments

#include <bmp.h>
#include <yuv.h>
#include <conversions.h>
#include <subsample_algo.h>
#include <byte.h>
#include <no_lsan_debug.h>

#include <iostream> // temp
#include <fstream>  // temp
#include <ios>      // temp, for std::hex manipulator
#include <string>
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
    BMP bmp{"/home/rj_rl/Desktop/work/pics/nice.bmp"};

    auto yuv444 = BMP_to_YUV444(bmp);
    //auto yuv420 = YUV444_to_YUV420(yuv444, Sample::mean<>);
    auto yuv420 = BMP_to_YUV420(bmp);

    ofstream{"/home/rj_rl/Desktop/work/output-mine/nice_mean.yuv", ios::binary}.write(
        reinterpret_cast<char*>(&yuv420.data[0]), yuv420.data.size()
    );

    YUV yuv{"/home/rj_rl/Desktop/work/pics/oddity.yuv", 3, 3, YUV::Type::Planar420};

    return 0;
}
