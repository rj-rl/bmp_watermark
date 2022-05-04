#include <yuv.h>

#include <fstream>
#include <utility>
#include <stdexcept>

using namespace std;
using namespace Utility;

YUV::YUV(const string& filename, size_t width, size_t height, Type type)
    : width{width}
    , height{height}
    , type{type}
{
    switch (type) {
    case Type::Planar444:
        data.resize(width * height * 3);
        break;
    case Type::Planar420:
        data.resize(width * height * 3 / 2);
        break;
    default:
        throw runtime_error{"Unsupported YUV format"};
    }

    ifstream file{filename, ios::binary | ios::in};
    if (!file) {
        throw runtime_error{"Could not open the YUV file"};
    }
    file.read(reinterpret_cast<char*>(&data[0]), data.size());
}

YUV::YUV(vector<byte_t> data, size_t width, size_t height, Type type)
    : width{width}
    , height{height}
    , type{type}
    , data{move(data)}
{}
