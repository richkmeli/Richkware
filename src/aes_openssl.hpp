#ifndef AES_OPENSSL_H
#define AES_OPENSSL_H

#include <string>
#include <vector>
#include <cstdint>

class AESCrypto {
public:
    // Encrypt plaintext with given key (SHA-256 derived)
    // Returns IV + ciphertext
    static std::vector<uint8_t> Encrypt(const std::vector<uint8_t>& plaintext, const std::vector<uint8_t>& key);
    
    // Decrypt ciphertext (expects IV prepended)
    // Returns plaintext
    static std::vector<uint8_t> Decrypt(const std::vector<uint8_t>& ciphertext, const std::vector<uint8_t>& key);
};

#endif
