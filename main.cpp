// TODO: assert(Размер bmp картинки может быть меньше либо равен размеру картинки входного видеоряда)
// TODO: parse command line arguments

#include <bmp.h>
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
    BMP bmp = BMP::from_file("/home/rj_rl/Desktop/work/pics/bw.bmp");

    auto yuv444 = BMP_to_YCbCr(bmp);
    ofstream{"/home/rj_rl/Desktop/work/test-output/bw444.yuv"}.write(
        reinterpret_cast<char*>(&yuv444[0]), yuv444.size()
    );

    cout << hex;
    auto line_sz = bmp.info_header.width_px * 3;
    size_t i = 0u;
    for (auto chunk : yuv444) {
        cout.width(2);
        cout << +chunk << ' ';
        ++i;
        if (i % 3 == 0) cout << ' ';
        if (i % line_sz == 0) cout << '\n';
    }
    cout << '\n';

    return 0;
}
