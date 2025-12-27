#include "../src/aes.hpp"
#include <iostream>
#include <vector>
#include <iomanip>

int main() {
    // NIST AES-256-ECB Vector
    // Key: 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f
    std::vector<uint8_t> key = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
    };
    
    // Plaintext: 00112233445566778899aabbccddeeff
    std::vector<uint8_t> plaintext = {
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
    };
    
    AES aes(key);
    // Encrypt (CBC with 0 IV = ECB for first block)
    // Note: Encrypt adds padding, so output will be > 16 bytes.
    // We only check first 16 bytes (IV) and next 16 bytes (Ciphertext block 1).
    // Wait, AES::Encrypt prepends IV.
    // So output[0..15] is IV (zeros).
    // output[16..31] is First Block Ciphertext.
    
    std::vector<uint8_t> encrypted = aes.Encrypt(plaintext);
    
    std::cout << "Ciphertext (Block 1): ";
    for(int i = 16; i < 32; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)encrypted[i];
    }
    std::cout << std::endl;
    
    // Expected: 8ea2b7ca516745bfeafc49904b496089
    
    return 0;
}
