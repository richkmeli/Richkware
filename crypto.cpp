/*
*      Copyright 2016 Riccardo Melioli. All Rights Reserved.
*/

#include "crypto.h"

std::string Encrypt(std::string input, std::string key) {
    std::string output;

    for (std::string::iterator it = input.begin(), itk = key.begin() ; it != input.end(); ++it, ++itk) {
        char c = *itk;
        output += (*it) ^ c;

        if(itk == key.end()) itk = key.begin();
    }

    return output;
}

std::string Decrypt(std::string input, std::string key) {
    std::string output;

    for (std::string::iterator it = input.begin(), itk = key.begin() ; it != input.end(); ++it, ++itk) {
        char c = *itk;
        output += (*it) ^ c;

        if(itk == key.end()) itk = key.begin();
    }

    return output;
}