//
// Created by bladez on 12/06/19.
//

#include <iostream>
#include "utils.h"

static std::vector<std::string> utils::split(std::string s, std::string delimiter) {
    std::vector<std::string> result;

    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        result.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    return result;
}
