#include <bmp.h>
#include <string>
#include <fstream>
#include <stdexcept>

using namespace std;
using byte_t = uint8_t;

BMP::BMP(const string& filename)
{
    ifstream file{ "filename", ios::binary | ios::in };
    if (!file) {
        throw runtime_error{ "Could not open the .bmp" };
    }

    // TODO: write constructing a BMP from file
    
}
