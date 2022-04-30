// TODO: assert(Размер bmp картинки может быть меньше либо равен размеру картинки входного видеоряда)

#include <fstream>  // temp
#include <iostream> // temp
#include <ios>      // temp, for std::hex manipulator
#include <string>
#include <vector>
#include <cstdint>
#include <cassert>
#include "no_lsan_debug.h"
#include <bmp.h>

using namespace std;

int main(int argc, const char* argv[])
{
    BMP bmp = BMP::from_file("/home/rj_rl/Desktop/work/pics/multi84.bmp");
    cout << hex;
    auto line_sz = bmp.padded_width;
    size_t i = 0u;
    for (auto chunk : bmp.pixel_data) {
        cout.width(2);
        cout << +chunk << ' ';
        ++i;
        if (i % 3 == 0) cout << ' ';
        if (i % line_sz == 0) cout << '\n';
    }
    cout << '\n';

    return 0;
}
