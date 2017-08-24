/*
*      Copyright 2016 Riccardo Melioli. All Rights Reserved.
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

class Blowfish {
public:
    Blowfish(){};
    Blowfish(const std::string &key);

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

class Crypto {
private:
    std::string encryptionKey;
    Blowfish blowfish;
public:
    Crypto() {}
    Crypto(const std::string &encryptionKeyArg);
    // get EncryptionKey value from RichkwareManagerServer (asymmetric)
    Crypto(const char* serverAddress, const char* port);
    Crypto& operator=(const Crypto& crypto);

    std::string Encrypt(std::string plaintext);
    std::string Decrypt(std::string ciphertext);

    const std::string &getEncryptionKey() const;
};

std::string Base64_encode(unsigned char const *, unsigned int len);
std::string Base64_decode(std::string const &s);

#endif /* CRYPTO_H_ */
