// TODO: Размер bmp картинки может быть меньше либо равен размеру картинки входного видеоряда
// TODO: parse command line arguments

#include <bmp.h>
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
    BMP bmp = BMP::from_file("/home/rj_rl/Desktop/work/pics/oddity.bmp");

    auto yuv444 = BMP_to_YUV444(bmp);
    auto yuv420 = YUV444_to_YUV420(yuv444, Subsample::median<>);

    ofstream{"/home/rj_rl/Desktop/work/output-mine/oddity_median.yuv", ios::binary}.write(
        reinterpret_cast<char*>(&yuv420[0]), yuv420.size()
    );







    
    bool needs_print = false;
    if (needs_print) {
        cout << hex;
        auto line_sz = bmp.info_header.width * 3;
        size_t i = 0u;
        for (auto chunk : yuv420) {
            cout.width(2);
            cout << +chunk << ' ';
            ++i;
            if (i % 3 == 0) cout << ' ';
            if (i % line_sz == 0) cout << '\n';
        }
        cout << '\n';
    }
    return 0;
}
