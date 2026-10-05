#include "LZ78.h"

// ---------- Dictionary ----------

Dictionary::Dictionary() {
    capacity = 8;
    count = 0;
    entries = new DictionaryEntry*[capacity];
}

Dictionary::~Dictionary() {
    for (int i = 0; i < count; i++) {
        delete[] entries[i]->data;
        delete entries[i];
    }
    delete[] entries;
}

void Dictionary::grow() {
    int newCapacity = capacity * 2;
    DictionaryEntry** newArray = new DictionaryEntry*[newCapacity];
    for (int i = 0; i < count; i++) {
        newArray[i] = entries[i];
    }
    delete[] entries; // only the pointer array; entries are still in use
    entries = newArray;
    capacity = newCapacity;
}

int Dictionary::add(const char* data, int length) {
    if (count == capacity) {
        grow();
    }
    DictionaryEntry* newEntry = new DictionaryEntry;
    newEntry->data = new char[length];
    for (int i = 0; i < length; i++) {
        newEntry->data[i] = data[i];
    }
    newEntry->length = length;

    entries[count] = newEntry;
    count++;
    return count;
}

int Dictionary::findMatch(const char* prefix, int length) const {
    for (int i = 0; i < count; i++) {
        if (entries[i]->length != length) continue;
        bool equal = true;
        for (int j = 0; j < length; j++) {
            if (entries[i]->data[j] != prefix[j]) { equal = false; break; }
        }
        if (equal) return i + 1;
    }
    return 0;
}

const DictionaryEntry* Dictionary::get(int oneBasedIndex) const {
    if (oneBasedIndex < 1 || oneBasedIndex > count) {
        throw LZ78InvalidFormatException("Corrupted data: dictionary index out of range");
    }
    return entries[oneBasedIndex - 1];
}

// ---------- LZ78 ----------
//
// End-of-text edge case: if the longest match reaches the end of the input,
// there is no next symbol, so symbol '\0' is emitted to mean "copy the phrase
// as is". The input is assumed not to contain real '\0' bytes (plain text).

void compressLZ78(const char* text, long length, Pair*& pairs, long& pairCount) {
    if (length <= 0) {
        pairs = nullptr;
        pairCount = 0;
        return;
    }

    Dictionary dictionary;

    long pairCapacity = 8;
    long used = 0;
    Pair* output = new Pair[pairCapacity];

    long i = 0;
    while (i < length) {
        // Find the longest dictionary match starting at text[i].
        int bestIndex = 0;
        long bestLength = 0;
        while (i + bestLength + 1 <= length) {
            long candidateLength = bestLength + 1;
            int idx = dictionary.findMatch(text + i, candidateLength);
            if (idx == 0) break;
            bestIndex = idx;
            bestLength = candidateLength;
        }

        char nextSymbol;
        long newPhraseLength;
        if (i + bestLength < length) {
            nextSymbol = text[i + bestLength];
            newPhraseLength = bestLength + 1;
        } else {
            nextSymbol = '\0'; // match reached the end of the text
            newPhraseLength = bestLength;
        }

        if (used == pairCapacity) {
            pairCapacity *= 2;
            Pair* newArray = new Pair[pairCapacity];
            for (long j = 0; j < used; j++) newArray[j] = output[j];
            delete[] output;
            output = newArray;
        }

        output[used].index = bestIndex;
        output[used].symbol = nextSymbol;
        used++;

        dictionary.add(text + i, newPhraseLength);
        i += newPhraseLength;
    }

    pairs = output;
    pairCount = used;
}

void decompressLZ78(const Pair* pairs, long pairCount, char*& outputText, long& outputLength) {
    if (pairCount <= 0) {
        outputText = nullptr;
        outputLength = 0;
        return;
    }

    Dictionary dictionary;

    long capacity = 16;
    long used = 0;
    char* buffer = new char[capacity];

    for (long p = 0; p < pairCount; p++) {
        int idx = pairs[p].index;
        char symbol = pairs[p].symbol;

        long entryLength = 0;
        const char* entryData = nullptr;
        if (idx != 0) {
            if (idx < 1 || idx > dictionary.getCount()) {
                delete[] buffer; // free before get() throws
            }
            const DictionaryEntry* entry = dictionary.get(idx);
            entryLength = entry->length;
            entryData = entry->data;
        }

        long phraseLength;
        if (idx == 0) {
            phraseLength = 1;
        } else if (symbol == '\0') {
            phraseLength = entryLength;
        } else {
            phraseLength = entryLength + 1;
        }

        while (used + phraseLength > capacity) {
            capacity *= 2;
            char* newBuffer = new char[capacity];
            for (long j = 0; j < used; j++) newBuffer[j] = buffer[j];
            delete[] buffer;
            buffer = newBuffer;
        }

        long phraseStart = used;
        if (idx != 0) {
            for (long j = 0; j < entryLength; j++) {
                buffer[used++] = entryData[j];
            }
        }
        if (idx == 0 || symbol != '\0') {
            buffer[used++] = symbol;
        }

        // Rebuild the dictionary exactly as during compression so indices match.
        dictionary.add(buffer + phraseStart, phraseLength);
    }

    outputText = buffer;
    outputLength = used;
}
