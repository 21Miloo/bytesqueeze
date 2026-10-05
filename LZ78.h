#ifndef LZ78_H
#define LZ78_H

#include <stdexcept>
#include <string>

// Thrown when decompressLZ78 finds a dictionary index out of range
// (corrupted or malformed data).
class LZ78InvalidFormatException : public std::runtime_error {
public:
    explicit LZ78InvalidFormatException(const std::string& detail)
        : std::runtime_error("LZ78: invalid format - " + detail) {}
};

// One output token: (dictionary index, next symbol).
// index == 0 means "no previous match" (new 1-character phrase).
// symbol == '\0' means "no extra symbol" (end-of-text edge case).
struct Pair {
    int index;
    char symbol;
};

// A phrase already seen, stored as a raw char array (not null-terminated).
struct DictionaryEntry {
    char* data;
    int length;
};

// Hand-made dynamic dictionary (no STL): an array of pointers to entries
// that doubles its capacity when full.
class Dictionary {
private:
    DictionaryEntry** entries;
    int count;
    int capacity;

    void grow();

public:
    Dictionary();
    ~Dictionary();

    // Copies the phrase and returns its 1-based index.
    int add(const char* data, int length);

    // Returns the 1-based index of the entry equal to (prefix, length), or 0.
    int findMatch(const char* prefix, int length) const;

    int getCount() const { return count; }
    const DictionaryEntry* get(int oneBasedIndex) const;
};

// Compresses 'text' with LZ78. Allocates 'pairs' with new[] (caller frees).
void compressLZ78(const char* text, long length, Pair*& pairs, long& pairCount);

// Rebuilds the original text. Allocates 'outputText' with new[] (caller frees).
void decompressLZ78(const Pair* pairs, long pairCount, char*& outputText, long& outputLength);

#endif // LZ78_H
