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

#endif /* CRYPTO_H_ */
