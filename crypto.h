/*
*      Copyright 2016 Riccardo Melioli. All Rights Reserved.
*/

#ifndef CRYPTO_H_
#define CRYPTO_H_

#include <string>
#include <iterator>

std::string EncryptDecrypt(std::string input, const char* key);

#endif /* CRYPTO_H_ */
