#include "Serialization.h"

using namespace std;

void pairsToBytes(const Pair* pairs, long pairCount, unsigned char*& bytes, long& length) {
    length = pairCount * 3;
    bytes = new unsigned char[length > 0 ? length : 1];

    for (long p = 0; p < pairCount; p++) {
        int index = pairs[p].index;
        if (index < 0 || index > 0xFFFF) {
            delete[] bytes;
            bytes = nullptr;
            throw SerializationException("index " + to_string(index)
                                         + " does not fit in 2 bytes (text too large for LZ78)");
        }
        bytes[3 * p]     = static_cast<unsigned char>((index >> 8) & 0xFF);
        bytes[3 * p + 1] = static_cast<unsigned char>(index & 0xFF);
        bytes[3 * p + 2] = static_cast<unsigned char>(pairs[p].symbol);
    }
}

void bytesToPairs(const unsigned char* bytes, long length, Pair*& pairs, long& pairCount) {
    if (length % 3 != 0) {
        throw SerializationException("Corrupted data: LZ78 data size (" + to_string(length)
                                     + " bytes) is not a multiple of 3");
    }
    pairCount = length / 3;
    pairs = new Pair[pairCount > 0 ? pairCount : 1];

    for (long p = 0; p < pairCount; p++) {
        pairs[p].index = (bytes[3 * p] << 8) | bytes[3 * p + 1];
        pairs[p].symbol = static_cast<char>(bytes[3 * p + 2]);
    }
}

void rleToBytes(const string& compressed, unsigned char*& bytes, long& length) {
    length = static_cast<long>(compressed.size());
    bytes = new unsigned char[length > 0 ? length : 1];
    for (long i = 0; i < length; i++) {
        bytes[i] = static_cast<unsigned char>(compressed[i]);
    }
}

string bytesToRLE(const unsigned char* bytes, long length) {
    return string(reinterpret_cast<const char*>(bytes), length);
}
