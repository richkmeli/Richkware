#pragma once

#include "../core/types.hpp"
#include <string>
#include <memory>
#include <vector>
#include <cstdint>

namespace richkware::crypto {

/**
 * @brief Supported cipher algorithms
 */
enum class CipherType {
    AES_256_GCM,
    ChaCha20_Poly1305
};

/**
 * @brief Supported key derivation functions
 */
enum class KdfType {
    PBKDF2_SHA256,
    Argon2id
};

/**
 * @brief Cipher interface for encryption/decryption operations
 */
class ICipher {
public:
    virtual ~ICipher() = default;
    
    /**
     * @brief Encrypt plaintext data
     * @param plaintext Data to encrypt
     * @param key Encryption key
     * @return Encrypted data or error
     */
    [[nodiscard]] virtual core::Result<core::Bytes> encrypt(
        const std::vector<std::uint8_t>& plaintext,
        const std::vector<std::uint8_t>& key) const = 0;
    
    /**
     * @brief Decrypt ciphertext data
     * @param ciphertext Data to decrypt
     * @param key Decryption key
     * @return Decrypted data or error
     */
    [[nodiscard]] virtual core::Result<core::Bytes> decrypt(
        const std::vector<std::uint8_t>& ciphertext,
        const std::vector<std::uint8_t>& key) const = 0;
    
    /**
     * @brief Get required key size in bytes
     * @return Key size
     */
    [[nodiscard]] virtual std::size_t key_size() const noexcept = 0;
};

/**
 * @brief AES-256-GCM cipher implementation
 */
class AesGcmCipher : public ICipher {
public:
    AesGcmCipher();
    ~AesGcmCipher() override;
    
    [[nodiscard]] core::Result<core::Bytes> encrypt(
        const std::vector<std::uint8_t>& plaintext,
        const std::vector<std::uint8_t>& key) const override;
    
    [[nodiscard]] core::Result<core::Bytes> decrypt(
        const std::vector<std::uint8_t>& ciphertext,
        const std::vector<std::uint8_t>& key) const override;
    
    [[nodiscard]] std::size_t key_size() const noexcept override { return 32; }

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

/**
 * @brief Key derivation interface
 */
class IKeyDerivation {
public:
    virtual ~IKeyDerivation() = default;
    
    /**
     * @brief Derive key from password
     * @param password Input password
     * @param salt Salt value
     * @param iterations Number of iterations
     * @param key_length Desired key length
     * @return Derived key or error
     */
    [[nodiscard]] virtual core::Result<core::Bytes> derive_key(
        const std::string& password,
        const std::vector<std::uint8_t>& salt,
        std::uint32_t iterations,
        std::size_t key_length) const = 0;
};

/**
 * @brief PBKDF2-SHA256 key derivation
 */
class Pbkdf2KeyDerivation : public IKeyDerivation {
public:
    [[nodiscard]] core::Result<core::Bytes> derive_key(
        const std::string& password,
        const std::vector<std::uint8_t>& salt,
        std::uint32_t iterations,
        std::size_t key_length) const override;
};

/**
 * @brief Cipher manager for high-level crypto operations
 */
class CipherManager {
public:
    /**
     * @brief Construct with cipher and KDF types
     * @param cipher_type Cipher algorithm to use
     * @param kdf_type Key derivation function to use
     */
    CipherManager(CipherType cipher_type = CipherType::AES_256_GCM,
                  KdfType kdf_type = KdfType::PBKDF2_SHA256);
    
    ~CipherManager();
    
    // Non-copyable, movable
    CipherManager(const CipherManager&) = delete;
    CipherManager& operator=(const CipherManager&) = delete;
    CipherManager(CipherManager&&) = default;
    CipherManager& operator=(CipherManager&&) = default;
    
    /**
     * @brief Set master password for key derivation
     * @param password Master password
     * @return Result indicating success or failure
     */
    [[nodiscard]] core::Result<void> set_password(const std::string& password);
    
    /**
     * @brief Encrypt string data
     * @param plaintext String to encrypt
     * @return Encrypted data as base64 string or error
     */
    [[nodiscard]] core::Result<std::string> encrypt_string(const std::string& plaintext);
    
    /**
     * @brief Decrypt string data
     * @param ciphertext Base64 encrypted string
     * @return Decrypted string or error
     */
    [[nodiscard]] core::Result<std::string> decrypt_string(const std::string& ciphertext);
    
    /**
     * @brief Generate secure random bytes
     * @param size Number of bytes to generate
     * @return Random bytes or error
     */
    [[nodiscard]] static core::Result<core::Bytes> generate_random(std::size_t size);

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace richkware::crypto