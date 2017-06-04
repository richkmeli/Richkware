/*
*      Copyright 2016 Riccardo Melioli. All Rights Reserved.
*/

#ifndef CRYPTO_H_
#define CRYPTO_H_

#include <string>
#include <iterator>
#include <cstdlib>

std::string Encrypt(std::string input, std::string key);
std::string Decrypt(std::string input, std::string key);

std::string Base64_encode(unsigned char const* , unsigned int len);
std::string Base64_decode(std::string const& s);

#endif /* CRYPTO_H_ */
