#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <stdexcept>
#include <string>
#include "LZ78.h"

// Converts each compressor's output to a flat byte array (so it can be
// encrypted and written to a file) and back. Both use 3-byte records:
//   RLE:  [count high][count low][symbol]
//   LZ78: [index high][index low][symbol]
// 'bytes' arrays are allocated with new[]; the caller frees them with delete[].

class SerializationException : public std::runtime_error {
public:
    explicit SerializationException(const std::string& detail)
        : std::runtime_error("Serialization: " + detail) {}
};

// ---- LZ78 ----
// Throws SerializationException if an index does not fit in 2 bytes (> 65535).
void pairsToBytes(const Pair* pairs, long pairCount, unsigned char*& bytes, long& length);
// Throws SerializationException if 'length' is not a multiple of 3.
void bytesToPairs(const unsigned char* bytes, long length, Pair*& pairs, long& pairCount);

// ---- RLE ----
// compressRLE already produces 3-byte records inside a std::string;
// these only copy them to/from a byte array.
void rleToBytes(const std::string& compressed, unsigned char*& bytes, long& length);
std::string bytesToRLE(const unsigned char* bytes, long length);

#endif // SERIALIZATION_H
