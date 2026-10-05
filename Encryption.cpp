#include "Encryption.h"

static void validateRotation(int n) {
    if (n <= 0 || n >= 8) {
        throw EncryptionInvalidParameterException("Encryption: n must satisfy 0 < n < 8");
    }
}

unsigned char rotateLeft(unsigned char byte, int n) {
    // Bits shifted out on the left re-enter on the right.
    return static_cast<unsigned char>((byte << n) | (byte >> (8 - n)));
}

unsigned char rotateRight(unsigned char byte, int n) {
    return static_cast<unsigned char>((byte >> n) | (byte << (8 - n)));
}

void encrypt(unsigned char* data, long length, int n, unsigned char key) {
    validateRotation(n);
    for (long i = 0; i < length; i++) {
        data[i] = rotateLeft(data[i], n) ^ key;
    }
}

void decrypt(unsigned char* data, long length, int n, unsigned char key) {
    validateRotation(n);
    for (long i = 0; i < length; i++) {
        data[i] = rotateRight(data[i] ^ key, n);
    }
}
