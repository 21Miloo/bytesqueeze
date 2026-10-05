#include <string>
#include "RLE.h"

using namespace std;

string compressRLE(const string& text) {
    string result;
    if (text.empty()) return result;

    size_t i = 0;
    while (i < text.size()) {
        char symbol = text[i];
        size_t count = 1;
        while (i + count < text.size() && text[i + count] == symbol && count < RLE_MAX_RUN_LENGTH) {
            count++;
        }

        result += static_cast<char>((count >> 8) & 0xFF); // high byte
        result += static_cast<char>(count & 0xFF);        // low byte
        result += symbol;
        i += count;
    }

    return result;
}

string descompressRLE(const string& compressed) {
    // Every record is exactly 3 bytes.
    if (compressed.size() % 3 != 0) {
        throw RLEInvalidFormatException("Corrupted data: size (" + to_string(compressed.size()) + " bytes) is not a multiple of 3");
    }

    string result;
    for (size_t i = 0; i < compressed.size(); i += 3) {
        unsigned int highByte = static_cast<unsigned char>(compressed[i]);
        unsigned int lowByte = static_cast<unsigned char>(compressed[i + 1]);
        unsigned int runLength = (highByte << 8) | lowByte;
        if (runLength == 0) {
            throw RLEInvalidFormatException("Corrupted data: run length 0 in record " + to_string(i / 3));
        }
        result.append(runLength, compressed[i + 2]);
    }

    return result;
}
