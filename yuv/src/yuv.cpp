#include <yuv.h>

#include <fstream>
#include <filesystem>
#include <utility>
#include <stdexcept>

using namespace std;
using namespace Utility;

YUV::YUV(const string& filename, size_t width, size_t height, Type type)
    : width {width}
    , height{height}
    , type  {type}
{
    ifstream file{filename, ios::binary | ios::in};
    if (!file) {
        throw runtime_error{"Could not open the YUV file"};
    }

    auto file_size = filesystem::file_size(filename);
    data.resize(file_size);
    file.read(reinterpret_cast<char*>(&data[0]), file_size);
}

YUV::YUV(vector<byte_t> data, size_t width, size_t height, Type type)
    : width {width}
    , height{height}
    , type  {type}
    , data  {move(data)}
{}

size_t YUV::frame_count() const
{
    switch (type) {
    case YUV::Type::Planar444:
        return data.size() / (3 * width * height);
    case YUV::Type::Planar420:
        return data.size() / (width * height + chroma_count_420(width, height));
    default:
        throw runtime_error{"Invalid YUV type"};
    }
}
