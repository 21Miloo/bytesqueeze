// ByteSqueeze - Informática II, Universidad de Antioquia
// Authors: Juan Camilo Lopez Castrillon, Juan David Acevedo

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include "RLE.h"
#include "LZ78.h"
#include "Serialization.h"
#include "Encryption.h"

using namespace std;

const int PATH_SIZE = 260;
const int METHOD_RLE = 1;
const int METHOD_LZ78 = 2;
const char ENCRYPTED_FILE[] = "encrypted.txt"; // raw encrypted bytes, not readable text
const char OUTPUT_FILE[] = "decrypted.txt";

// ---------- Files ----------

// Reads the whole file into 'data' (allocated with new[]).
// Binary mode so no byte is altered.
void readFile(const char* path, unsigned char*& data, long& length) {
    ifstream file(path, ios::binary);
    if (!file.is_open()) {
        throw runtime_error("could not open file '" + string(path) + "'");
    }

    file.seekg(0, ios::end);
    length = static_cast<long>(file.tellg());
    file.seekg(0, ios::beg);

    data = new unsigned char[length > 0 ? length : 1];
    file.read(reinterpret_cast<char*>(data), length);
}

// Creates or overwrites the file with 'length' bytes of 'data'.
void writeFile(const char* path, const char* data, long length) {
    ofstream file(path, ios::binary);
    if (!file.is_open()) {
        throw runtime_error("could not create file '" + string(path) + "'");
    }
    file.write(data, length);
}

// ---------- Pipeline ----------

// Original text -> compress with the chosen method -> byte array.
void compressToBytes(int method, const unsigned char* text, long length,
                     unsigned char*& bytes, long& bytesLength) {
    if (method == METHOD_RLE) {
        string compressed = compressRLE(string(reinterpret_cast<const char*>(text), length));
        rleToBytes(compressed, bytes, bytesLength);
    } else {
        Pair* pairs = nullptr;
        long pairCount = 0;
        compressLZ78(reinterpret_cast<const char*>(text), length, pairs, pairCount);
        try {
            pairsToBytes(pairs, pairCount, bytes, bytesLength);
        } catch (...) {
            delete[] pairs;
            throw;
        }
        delete[] pairs;
    }
}

// Decrypted byte array -> decompress -> original text.
void decompressFromBytes(int method, const unsigned char* bytes, long bytesLength,
                         char*& text, long& textLength) {
    if (method == METHOD_RLE) {
        string decompressed = descompressRLE(bytesToRLE(bytes, bytesLength));
        textLength = static_cast<long>(decompressed.size());
        text = new char[textLength > 0 ? textLength : 1];
        for (long i = 0; i < textLength; i++) {
            text[i] = decompressed[i];
        }
    } else {
        Pair* pairs = nullptr;
        long pairCount = 0;
        bytesToPairs(bytes, bytesLength, pairs, pairCount);
        try {
            decompressLZ78(pairs, pairCount, text, textLength);
        } catch (...) {
            delete[] pairs;
            throw;
        }
        delete[] pairs;
    }
}

// ---------- User input ----------

// Reads an integer and discards the rest of the line, so the next
// getline does not read an empty line.
int readInt(const char* prompt) {
    int value;
    cout << prompt;
    cin >> value;
    if (cin.fail()) {
        cin.clear();
        cin.ignore(1000, '\n');
        throw invalid_argument("you must enter a number");
    }
    cin.ignore(1000, '\n');
    return value;
}

int readMethod() {
    int method = readInt("Compression method (1 = RLE, 2 = LZ78): ");
    if (method != METHOD_RLE && method != METHOD_LZ78) {
        throw invalid_argument("method must be 1 (RLE) or 2 (LZ78)");
    }
    return method;
}

// Reads and validates n and K before any memory is allocated.
void readKey(int& n, int& key) {
    n = readInt("Bits to rotate n (1-7): ");
    if (n <= 0 || n >= 8) {
        throw invalid_argument("n must be between 1 and 7");
    }
    key = readInt("Key K (0-255): ");
    if (key < 0 || key > 255) {
        throw invalid_argument("key must be between 0 and 255");
    }
}

// ---------- Menu options ----------

// Option 1: read text file -> compress -> encrypt -> save ENCRYPTED_FILE.
void encryptFile() {
    char inputPath[PATH_SIZE];
    cout << "Text file to encrypt: ";
    cin.getline(inputPath, PATH_SIZE);

    int method = readMethod();
    int n, key;
    readKey(n, key);

    // Start at nullptr so the catch block can always free them safely.
    unsigned char* original = nullptr;
    long originalLength = 0;
    unsigned char* bytes = nullptr;
    long bytesLength = 0;

    try {
        readFile(inputPath, original, originalLength);

        compressToBytes(method, original, originalLength, bytes, bytesLength);
        cout << "\nOriginal size:   " << originalLength << " bytes\n";
        cout << "Compressed size: " << bytesLength << " bytes\n";

        encrypt(bytes, bytesLength, n, static_cast<unsigned char>(key));
        writeFile(ENCRYPTED_FILE, reinterpret_cast<const char*>(bytes), bytesLength);
        cout << "Encrypted file saved to: " << ENCRYPTED_FILE << "\n";
    } catch (...) {
        delete[] original;
        delete[] bytes;
        throw; // main prints the error
    }

    delete[] original;
    delete[] bytes;
}

// Option 2: read encrypted file -> decrypt -> decompress -> save OUTPUT_FILE.
// Must use the same method, n and K that were used to encrypt.
void decryptFile() {
    char inputPath[PATH_SIZE];
    cout << "Encrypted file: ";
    cin.getline(inputPath, PATH_SIZE);

    int method = readMethod();
    int n, key;
    readKey(n, key);

    unsigned char* bytes = nullptr;
    long bytesLength = 0;
    char* text = nullptr;
    long textLength = 0;

    try {
        readFile(inputPath, bytes, bytesLength);
        decrypt(bytes, bytesLength, n, static_cast<unsigned char>(key));

        decompressFromBytes(method, bytes, bytesLength, text, textLength);
        writeFile(OUTPUT_FILE, text, textLength);
        cout << "\nRecovered size: " << textLength << " bytes\n";
        cout << "Decrypted text saved to: " << OUTPUT_FILE << "\n";
    } catch (...) {
        delete[] bytes;
        delete[] text;
        throw;
    }

    delete[] bytes;
    delete[] text;
}

// ---------- Main ----------

int main() {
    int option;

    do {
        cout << "\n===== ByteSqueeze: Compression and encryption =====\n";
        cout << "1. Compress and encrypt a file\n";
        cout << "2. Decrypt and decompress a file\n";
        cout << "0. Exit\n";
        cout << "Option: ";
        cin >> option;

        if (cin.fail()) {
            cin.clear();
            option = -1;
        }
        cin.ignore(1000, '\n');

        try {
            if (option == 1) {
                encryptFile();
            } else if (option == 2) {
                decryptFile();
            } else if (option == 0) {
                cout << "Exiting...\n";
            } else {
                cout << "Invalid option.\n";
            }
        } catch (const exception& e) {
            cout << "Error: " << e.what() << "\n";
        }
    } while (option != 0);

    return 0;
}
