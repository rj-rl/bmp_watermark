// enables use of asan under gdb (lsan barks at you otherwise)
extern "C" int __lsan_is_turned_off() { return 1; }

#include <fstream>
#include <iostream>
#include <ios>  // for std::hex manipulator
#include <string>
#include <vector>
#include <cstdint>
#include <cassert>
#include <bmp.h>

using namespace std;
using byte_t = uint8_t;

// TODO: assert(Размер bmp картинки может быть меньше либо равен размеру картинки входного видеоряда)
// TODO: assert(compression method value is 0)

int main(int argc, const char* argv[])
{
    string filename{ "../bw.bmp" };
    ifstream file{ filename, ios_base::binary | ios_base::in };
    if (!file) cerr << "john is kill\n";

    BMPFileHeader header;
    // rip big-endian users
    file.read(reinterpret_cast<char*>(&header), sizeof(header));

    cout << "File type: " << header.file_type << '\n';
    cout << "File size: " << header.file_size << '\n';
    cout << "Data offset: " << header.data_offset << '\n';

    BMPInfoHeader info_header;
    file.read(reinterpret_cast<char*>(&info_header), sizeof(info_header));
    assert(info_header.compression == 0);

    cout << "Image width: " << info_header.width << '\n';
    cout << "Image height: " << info_header.height << '\n';
    cout << "Compression used: " << info_header.compression << '\n';
    
    // vector<byte_t> data;
    // byte_t chunk;
    // while (file >> noskipws >> chunk) {
    //     data.push_back(chunk);
    // }

    // //cout << hex;
    // for (auto chunk : data) {
    //     cout << +chunk << ' ';
    // }
    // cout << '\n';
    // cout << data.size() << '\n';

    return 0;
}
