#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <stdexcept>

// Thrown when the rotation amount does not satisfy 0 < n < 8.
class EncryptionInvalidParameterException : public std::invalid_argument {
public:
    explicit EncryptionInvalidParameterException(const char* detail)
        : std::invalid_argument(detail) {}
};

// Circular bit rotation of a single byte.
// Example: rotateLeft(01000001, 2) == 00000101
unsigned char rotateLeft(unsigned char byte, int n);
unsigned char rotateRight(unsigned char byte, int n);

// In-place encryption: rotate each byte n bits left, then XOR with key.
// Throws EncryptionInvalidParameterException if n is not in (0, 8).
void encrypt(unsigned char* data, long length, int n, unsigned char key);

// Inverse of encrypt: XOR with key, then rotate n bits right.
void decrypt(unsigned char* data, long length, int n, unsigned char key);

#endif // ENCRYPTION_H
