#include "richkware/crypto/cipher.hpp"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/kdf.h>
#include <openssl/buffer.h>
#include <vector>
#include <string>

namespace richkware::crypto {

// Helper to manage OpenSSL EVP_CIPHER_CTX
struct EvpCipherCtx {
    EVP_CIPHER_CTX* ctx;
    EvpCipherCtx() : ctx(EVP_CIPHER_CTX_new()) {}
    ~EvpCipherCtx() { EVP_CIPHER_CTX_free(ctx); }
};

// AES-GCM implementation
class AesGcmCipher::Impl {
public:
    core::Result<core::Bytes> encrypt(const std::vector<uint8_t>& plaintext, const std::vector<uint8_t>& key) const {
        if (key.size() != 32) {
            return core::RichkwareError{core::ErrorCode::CryptoError, "Invalid key size for AES-256-GCM"};
        }

        core::Bytes iv(12);
        if (RAND_bytes(iv.data(), static_cast<int>(iv.size())) != 1) {
            return core::RichkwareError{core::ErrorCode::CryptoError, "Failed to generate IV"};
        }

        EvpCipherCtx ctx;
        if (!ctx.ctx) {
            return core::RichkwareError{core::ErrorCode::CryptoError, "Failed to create EVP_CIPHER_CTX"};
        }

        if (1 != EVP_EncryptInit_ex(ctx.ctx, EVP_aes_256_gcm(), NULL, NULL, NULL)) {
            return core::RichkwareError{core::ErrorCode::CryptoError, "EVP_EncryptInit_ex failed"};
        }
        if (1 != EVP_EncryptInit_ex(ctx.ctx, NULL, NULL, key.data(), iv.data())) {
            return core::RichkwareError{core::ErrorCode::CryptoError, "EVP_EncryptInit_ex failed on key/iv"};
        }

        core::Bytes ciphertext(plaintext.size());
        int len;
        if (1 != EVP_EncryptUpdate(ctx.ctx, ciphertext.data(), &len, plaintext.data(), static_cast<int>(plaintext.size()))) {
            return core::RichkwareError{core::ErrorCode::CryptoError, "EVP_EncryptUpdate failed"};
        }

        int len2;
        if (1 != EVP_EncryptFinal_ex(ctx.ctx, ciphertext.data() + len, &len2)) {
            return core::RichkwareError{core::ErrorCode::CryptoError, "EVP_EncryptFinal_ex failed"};
        }
        ciphertext.resize(len + len2);

        core::Bytes tag(16);
        if (1 != EVP_CIPHER_CTX_ctrl(ctx.ctx, EVP_CTRL_GCM_GET_TAG, 16, tag.data())) {
            return core::RichkwareError{core::ErrorCode::CryptoError, "EVP_CTRL_GCM_GET_TAG failed"};
        }

        core::Bytes encrypted_data;
        encrypted_data.insert(encrypted_data.end(), iv.begin(), iv.end());
        encrypted_data.insert(encrypted_data.end(), tag.begin(), tag.end());
        encrypted_data.insert(encrypted_data.end(), ciphertext.begin(), ciphertext.end());

        return encrypted_data;
    }

    core::Result<core::Bytes> decrypt(const std::vector<uint8_t>& ciphertext, const std::vector<uint8_t>& key) const {
        if (key.size() != 32) {
            return core::RichkwareError{core::ErrorCode::CryptoError, "Invalid key size for AES-256-GCM"};
        }
        if (ciphertext.size() < 28) { // 12 (IV) + 16 (tag)
            return core::RichkwareError{core::ErrorCode::CryptoError, "Invalid ciphertext size"};
        }

        core::Bytes iv(ciphertext.begin(), ciphertext.begin() + 12);
        core::Bytes tag(ciphertext.begin() + 12, ciphertext.begin() + 28);
        core::Bytes encrypted_data(ciphertext.begin() + 28, ciphertext.end());

        EvpCipherCtx ctx;
        if (!ctx.ctx) {
            return core::RichkwareError{core::ErrorCode::CryptoError, "Failed to create EVP_CIPHER_CTX"};
        }

        if (1 != EVP_DecryptInit_ex(ctx.ctx, EVP_aes_256_gcm(), NULL, NULL, NULL)) {
            return core::RichkwareError{core::ErrorCode::CryptoError, "EVP_DecryptInit_ex failed"};
        }
        if (1 != EVP_DecryptInit_ex(ctx.ctx, NULL, NULL, key.data(), iv.data())) {
            return core::RichkwareError{core::ErrorCode::CryptoError, "EVP_DecryptInit_ex failed on key/iv"};
        }

        core::Bytes plaintext(encrypted_data.size());
        int len;
        if (1 != EVP_DecryptUpdate(ctx.ctx, plaintext.data(), &len, encrypted_data.data(), static_cast<int>(encrypted_data.size()))) {
            return core::RichkwareError{core::ErrorCode::CryptoError, "EVP_DecryptUpdate failed"};
        }

        if (1 != EVP_CIPHER_CTX_ctrl(ctx.ctx, EVP_CTRL_GCM_SET_TAG, 16, tag.data())) {
            return core::RichkwareError{core::ErrorCode::CryptoError, "EVP_CTRL_GCM_SET_TAG failed"};
        }

        int len2;
        if (1 != EVP_DecryptFinal_ex(ctx.ctx, plaintext.data() + len, &len2)) {
            return core::RichkwareError{core::ErrorCode::CryptoError, "Authentication failed: EVP_DecryptFinal_ex failed"};
        }
        plaintext.resize(len + len2);

        return plaintext;
    }
};

AesGcmCipher::AesGcmCipher() : pimpl_(std::make_unique<Impl>()) {}
AesGcmCipher::~AesGcmCipher() = default;

core::Result<core::Bytes> AesGcmCipher::encrypt(const std::vector<uint8_t>& plaintext, const std::vector<uint8_t>& key) const {
    return pimpl_->encrypt(plaintext, key);
}

core::Result<core::Bytes> AesGcmCipher::decrypt(const std::vector<uint8_t>& ciphertext, const std::vector<uint8_t>& key) const {
    return pimpl_->decrypt(ciphertext, key);
}

// Pbkdf2KeyDerivation implementation
core::Result<core::Bytes> Pbkdf2KeyDerivation::derive_key(const std::string& password, const std::vector<uint8_t>& salt, uint32_t iterations, std::size_t key_length) const {
    core::Bytes key(key_length);
    if (1 != PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.length()), salt.data(), static_cast<int>(salt.size()), iterations, EVP_sha256(), static_cast<int>(key_length), key.data())) {
        return core::RichkwareError{core::ErrorCode::CryptoError, "PKCS5_PBKDF2_HMAC failed"};
    }
    return key;
}


// Base64 encoding/decoding
std::string base64_encode(const core::Bytes& data) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, data.data(), static_cast<int>(data.size()));
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    std::string encoded(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);

    return encoded;
}

core::Result<core::Bytes> base64_decode(const std::string& data) {
    BIO *bio, *b64;
    core::Bytes decoded(data.size());
    int decoded_size = 0;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new_mem_buf(data.c_str(), data.length());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    decoded_size = BIO_read(bio, decoded.data(), static_cast<int>(decoded.size()));
    BIO_free_all(bio);

    if (decoded_size < 0) {
        return core::RichkwareError{core::ErrorCode::CryptoError, "Base64 decoding failed"};
    }

    decoded.resize(decoded_size);
    return decoded;
}

// CipherManager implementation
class CipherManager::Impl {
public:
    std::string password_;
    std::unique_ptr<ICipher> cipher_;
    std::unique_ptr<IKeyDerivation> kdf_;

    Impl(CipherType cipher_type, KdfType kdf_type) {
        if (cipher_type == CipherType::AES_256_GCM) {
            cipher_ = std::make_unique<AesGcmCipher>();
        }
        // Add other ciphers here
        else {
            // Default or error
            cipher_ = std::make_unique<AesGcmCipher>();
        }

        if (kdf_type == KdfType::PBKDF2_SHA256) {
            kdf_ = std::make_unique<Pbkdf2KeyDerivation>();
        }
        // Add other KDFs here
        else {
            // Default or error
            kdf_ = std::make_unique<Pbkdf2KeyDerivation>();
        }
    }
};

CipherManager::CipherManager(CipherType cipher_type, KdfType kdf_type) : pimpl_(std::make_unique<Impl>(cipher_type, kdf_type)) {}
CipherManager::~CipherManager() = default;

core::Result<void> CipherManager::set_password(const std::string& password) {
    if (password.empty()) {
        return core::RichkwareError{core::ErrorCode::InvalidArgument, "Password cannot be empty"};
    }
    pimpl_->password_ = password;
    return core::Result<void>{};
}

core::Result<std::string> CipherManager::encrypt_string(const std::string& plaintext) {
    if (pimpl_->password_.empty()) {
        return core::RichkwareError{core::ErrorCode::CryptoError, "Password not set"};
    }
    if (!pimpl_->cipher_ || !pimpl_->kdf_) {
        return core::RichkwareError{core::ErrorCode::CryptoError, "Cipher or KDF not initialized"};
    }

    auto salt_res = generate_random(16);
    if (!salt_res) {
        return salt_res.error();
    }
    core::Bytes salt = salt_res.value();
    
    auto key_res = pimpl_->kdf_->derive_key(pimpl_->password_, salt, 10000, pimpl_->cipher_->key_size());
    if(!key_res){
        return key_res.error();
    }
    core::Bytes key = key_res.value();

    core::Bytes plaintext_bytes(plaintext.begin(), plaintext.end());
    auto encrypted_res = pimpl_->cipher_->encrypt(plaintext_bytes, key);
    if (!encrypted_res) {
        return encrypted_res.error();
    }
    core::Bytes encrypted = encrypted_res.value();

    core::Bytes final_data;
    final_data.insert(final_data.end(), salt.begin(), salt.end());
    final_data.insert(final_data.end(), encrypted.begin(), encrypted.end());

    return base64_encode(final_data);
}

core::Result<std::string> CipherManager::decrypt_string(const std::string& ciphertext) {
    if (pimpl_->password_.empty()) {
        return core::RichkwareError{core::ErrorCode::CryptoError, "Password not set"};
    }
    if (!pimpl_->cipher_ || !pimpl_->kdf_) {
        return core::RichkwareError{core::ErrorCode::CryptoError, "Cipher or KDF not initialized"};
    }

    auto decoded_res = base64_decode(ciphertext);
    if (!decoded_res) {
        return decoded_res.error();
    }
    core::Bytes decoded = decoded_res.value();

    if (decoded.size() < 16) {
        return core::RichkwareError{core::ErrorCode::CryptoError, "Invalid encrypted data size"};
    }

    core::Bytes salt(decoded.begin(), decoded.begin() + 16);
    core::Bytes encrypted_data(decoded.begin() + 16, decoded.end());

    auto key_res = pimpl_->kdf_->derive_key(pimpl_->password_, salt, 10000, pimpl_->cipher_->key_size());
    if(!key_res){
        return key_res.error();
    }
    core::Bytes key = key_res.value();

    auto decrypted_res = pimpl_->cipher_->decrypt(encrypted_data, key);
    if (!decrypted_res) {
        return decrypted_res.error();
    }
    core::Bytes decrypted = decrypted_res.value();

    return std::string(decrypted.begin(), decrypted.end());
}

core::Result<core::Bytes> CipherManager::generate_random(std::size_t size) {
    core::Bytes result(size);
    if (RAND_bytes(result.data(), static_cast<int>(size)) != 1) {
        return core::RichkwareError{core::ErrorCode::CryptoError, "Failed to generate random bytes"};
    }
    return result;
}

} // namespace richkware::crypto