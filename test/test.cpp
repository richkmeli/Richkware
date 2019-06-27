
#include "../src/richkware.h"
#include <assert.h>


void TEST_Crypto();


int main() {

    std::cout << "Init test phase..." << std::endl;

    TEST_Crypto();

    std::cout << "Tests done successfully." << std::endl;
    system("pause");
    return 0;

}

void TEST_Crypto(){
    std::vector<std::string> strings = {"qwerty", "343wbt3wv3", "23v45.c-55,.v32c", "43344545tbv45v2c5223v4234", "34",
                                        "- - ", ".-,", "&"};

    for (const std::string &plaintext : strings) {
        for (const std::string &key : strings) {
            std::string res = RC4EncryptDecrypt(plaintext, key);
            std::string dec = RC4EncryptDecrypt(res, key);
            assert(plaintext == dec);
            //std::cout << plaintext << " " << dec << std::endl;

            res = Crypto::Encrypt(plaintext, key);
            dec = Crypto::Decrypt(res, key);
            assert(plaintext == dec);
            //std::cout << plaintext << " " << dec << std::endl;

            res = Base64_encode((const unsigned char *) plaintext.c_str(), plaintext.length());
            dec = Base64_decode(res);
            assert(plaintext == dec);
        }
    }
}
