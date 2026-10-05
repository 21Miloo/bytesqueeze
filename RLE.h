#ifndef RLE_H
#define RLE_H

#include <string>
#include <stdexcept>

// Thrown when descompressRLE receives malformed data
// (length not a multiple of 3, or a run length of 0).
class RLEInvalidFormatException : public std::runtime_error {
public:
    explicit RLEInvalidFormatException(const std::string& detail)
        : std::runtime_error("RLE: invalid format - " + detail) {}
};

// Largest run that fits in a record (2-byte count).
// Longer runs are split into several records.
const unsigned int RLE_MAX_RUN_LENGTH = 0xFFFF;

// Run-Length Encoding. Each run of equal characters becomes a 3-byte record:
// [count high][count low][symbol].
// Example: "AAAABBBCC" -> {0,4,'A'} {0,3,'B'} {0,2,'C'}
// The count is binary (not text like "4A3B") so the input may contain digits.
std::string compressRLE(const std::string& text);

// Reverses compressRLE. Throws RLEInvalidFormatException on malformed input.
std::string descompressRLE(const std::string& compressed);

#endif // RLE_H
