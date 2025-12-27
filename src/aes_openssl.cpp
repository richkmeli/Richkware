#include "aes_openssl.hpp"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <stdexcept>
#include <cstring>

std::vector<uint8_t> AESCrypto::Encrypt(const std::vector<uint8_t>& plaintext, const std::vector<uint8_t>& key) {
    if (key.size() != 32) {
        throw std::invalid_argument("Key must be 32 bytes for AES-256");
    }
    
    // Generate random IV
    unsigned char iv[16];
    if (!RAND_bytes(iv, 16)) {
        throw std::runtime_error("Failed to generate IV");
    }
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create cipher context");
    }
    
    // Initialize encryption
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize encryption");
    }
    
    // Allocate buffer for ciphertext (plaintext + block size for padding)
    std::vector<uint8_t> ciphertext(plaintext.size() + 16);
    int len = 0;
    int ciphertext_len = 0;
    
    // Encrypt
    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), plaintext.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Encryption failed");
    }
    ciphertext_len = len;
    
    // Finalize encryption (adds padding)
    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Encryption finalization failed");
    }
    ciphertext_len += len;
    
    EVP_CIPHER_CTX_free(ctx);
    
    // Prepend IV to ciphertext
    std::vector<uint8_t> result(16 + ciphertext_len);
    std::memcpy(result.data(), iv, 16);
    std::memcpy(result.data() + 16, ciphertext.data(), ciphertext_len);
    
    return result;
}

std::vector<uint8_t> AESCrypto::Decrypt(const std::vector<uint8_t>& ciphertext, const std::vector<uint8_t>& key) {
    if (key.size() != 32) {
        throw std::invalid_argument("Key must be 32 bytes for AES-256");
    }
    
    if (ciphertext.size() < 16) {
        throw std::invalid_argument("Ciphertext too short (missing IV)");
    }
    
    // Extract IV
    unsigned char iv[16];
    std::memcpy(iv, ciphertext.data(), 16);
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create cipher context");
    }
    
    // Initialize decryption
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize decryption");
    }
    
    // Allocate buffer for plaintext
    std::vector<uint8_t> plaintext(ciphertext.size());
    int len = 0;
    int plaintext_len = 0;
    
    // Decrypt
    if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data() + 16, ciphertext.size() - 16) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Decryption failed");
    }
    plaintext_len = len;
    
    // Finalize decryption (removes padding)
    if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Decryption failed (bad padding or key)");
    }
    plaintext_len += len;
    
    EVP_CIPHER_CTX_free(ctx);
    
    plaintext.resize(plaintext_len);
    return plaintext;
}
