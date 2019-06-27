
#include "../src/richkware.h"
#include <assert.h>
#include <iostream>


void TEST_Crypto();


int main() {
    //ShowWindow(GetConsoleWindow(), 1);
    std::cout << "Init test phase..." << std::endl << std::endl;

    try {
        TEST_Crypto();
    }
    catch (std::string e) {
        std::cout << "TEST Exception: " << e << std::endl;
    }


    std::cout << std::endl << "Tests done successfully." << std::endl;
    system("pause");
    return 0;

}

void test(std::string s1, std::string s2) {
    if (s1 == s2) {
        return;
    } else {
        throw s1 + " != " + s2;
    }
}

void test(bool b) {
    if (b) {
        return;
    } else {
        throw "boolean false";
    }
}

void TEST_Crypto() {
    std::vector<std::string> strings = {"qwerty", "343wbt3wv3", "23v45.c-55,.v32c", "43344545tbv45v2c5223v4234", "34",
                                        "- - ", ".-,", "&"};

    for (const std::string &plaintext : strings) {
        for (const std::string &key : strings) {
            std::string res = RC4EncryptDecrypt(plaintext, key);
            std::string dec = RC4EncryptDecrypt(res, key);
            //assert(plaintext == dec);
            test(plaintext, dec);
            //std::cout << plaintext << " " << dec << std::endl;

            res = Crypto::Encrypt(plaintext, key);
            dec = Crypto::Decrypt(res, key);
            //assert(plaintext == dec);
            test(plaintext, dec);
            //std::cout << plaintext << " " << dec << std::endl;

            res = Base64_encode((const unsigned char *) plaintext.c_str(), plaintext.length());
            dec = Base64_decode(res);
            assert(plaintext == dec);
        }
    }
}
