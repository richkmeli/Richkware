#include "../src/crypto.h"
#include "../src/sha256.hpp"
#include <iostream>
#include <string>
#include <vector>

int main() {
    Crypto crypto("mysecretkey");
    
    // Debug Key Hash
    {
        SHA256 sha;
        sha.update("mysecretkey");
        std::vector<uint8_t> k = sha.digest();
        std::cout << "KeyHash: ";
        for(auto b : k) printf("%02x", b);
        std::cout << std::endl;
    }

    std::string text = "Hello World From C++";
    std::string encrypted = crypto.Encrypt(text);
    std::cout << "Ciphertext: " << encrypted << std::endl;
    
    // Test decrypt
    std::string decrypted = crypto.Decrypt(encrypted);
    std::cout << "Decrypted: " << decrypted << std::endl;
    
    return 0;
}
