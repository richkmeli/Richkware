/*
*      Copyright 2016 Riccardo Melioli. All Rights Reserved.
*/

#include "crypto.h"

// XOR
std::string EncryptDecrypt(std::string input, const char* key) {
	int ikey = 5;/////////////////////////////////////////////
	std::string output;

	for (std::string::iterator it = input.begin(); it != input.end(); ++it) {
		output += (*it) ^ ikey;
	}

	return output;
}