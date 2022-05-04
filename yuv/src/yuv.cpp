#include <yuv.h>

#include <fstream>
#include <stdexcept>

using namespace std;

YUV::YUV(size_t width, size_t height, Type type)
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
    // FIXME: refactor 2 ctors into one, if filename can't open, return empty YUV
}

YUV YUV::from_file(const string& filename,
                   size_t width, size_t height, Type type)
{
    return YUV{filename, width, height, type};
}

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
