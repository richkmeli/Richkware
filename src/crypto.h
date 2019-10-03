/*
*      Copyright 2016 Riccardo Melioli.
*/

#ifndef CRYPTO_H_
#define CRYPTO_H_

#include <string>
#include <iterator>
#include <cstdlib>
#include <stdint.h>
#include <cstddef>
#include <vector>
#include <cstring>
#include <algorithm>
#include <stdexcept>

std::string RC4EncryptDecrypt(const std::string &pszText, const std::string &pszKey);

class Blowfish {
public:
    Blowfish() {};

    explicit Blowfish(const std::string &key);

    std::string Encrypt(const std::string &src) const;

    std::string Decrypt(const std::string &src) const;

private:
    void SetKey(const char *keyT, size_t byte_length);

    void EncryptBlock(uint32_t *left, uint32_t *right) const;

    void DecryptBlock(uint32_t *left, uint32_t *right) const;

    uint32_t Feistel(uint32_t value) const;

    uint32_t pary_[18];
    uint32_t sbox_[4][256];
};

std::string Vigenere(std::string input, std::string key);

class Crypto {
private:
    std::string encryptionKey;
public:
    Crypto() {}

    explicit Crypto(const std::string &encryptionKeyArg);

    // get EncryptionKey value from RichkwareManagerServer (asymmetric)
    Crypto(const char *serverAddress, const char *port);

    Crypto &operator=(const Crypto &crypto);

    std::string Encrypt(const std::string &plaintext);

    std::string Decrypt(const std::string &ciphertext);

    static std::string Encrypt(std::string plaintext, const std::string &encryptionKey);

    static std::string Decrypt(std::string ciphertext, const std::string &encryptionKey);

    const std::string &getEncryptionKey() const;
};

std::string Base64_urlencode(std::string string_to_encode);

std::string Base64_urldecode(std::string const &s);

std::string Base64_encode(const unsigned char *bytes_to_encode, unsigned int in_len);

std::string Base64_decode(std::string const &s);

std::string hex_to_string(const std::string &input);

std::string string_to_hex(const std::string &input);

#endif /* CRYPTO_H_ */
