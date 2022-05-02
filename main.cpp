// TODO: assert(Размер bmp картинки может быть меньше либо равен размеру картинки входного видеоряда)
// TODO: parse command line arguments

#include <bmp.h>
#include <conversions.h>
#include <byte.h>
#include <no_lsan_debug.h>

#include <iostream> // temp
#include <fstream>
#include <ios>      // temp, for std::hex manipulator
#include <string>
#include <vector>
#include <stdexcept>

using namespace std;

int main(int argc, const char* argv[])
{
    BMP bmp = BMP::from_file("/home/rj_rl/Desktop/work/pics/multi84.bmp");

    auto yuv420 = BMP_to_YUV420(bmp);
    ofstream{"/home/rj_rl/Desktop/work/test-output/multi84_420.yuv"}.write(
        reinterpret_cast<char*>(&yuv420[0]), yuv420.size()
    );

    cout << hex;
    auto line_sz = bmp.info_header.width_px * 3;
    size_t i = 0u;
    for (auto chunk : yuv420) {
        cout.width(2);
        cout << +chunk << ' ';
        ++i;
        if (i % 3 == 0) cout << ' ';
        if (i % line_sz == 0) cout << '\n';
    }
    cout << '\n';

    return 0;
}
