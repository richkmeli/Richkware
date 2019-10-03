//
// Created by bladez on 12/06/19.
//

#ifndef RICHKWARE_UTILS_H
#define RICHKWARE_UTILS_H


#include <string>
#include <vector>

class utils {
public:
    static std::vector<std::string> split(std::string s, std::string delimiter);

    static std::string concatVector(std::vector<std::string> vec, std::string delimiter = "");
};


#endif //RICHKWARE_UTILS_H
