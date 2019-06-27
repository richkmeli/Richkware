//
// Created by bladez on 12/06/19.
//

#include <iostream>
#include "utils.h"

std::vector<std::string> utils::split(std::string s, std::string delimiter) {
    std::vector<std::string> result;

    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        result.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    result.push_back(s);
    return result;
}

std::string utils::concatVector(std::vector<std::string> vec, std::string delimiter) {
    std::string accumulator = "";
    for (size_t i = 0; i < vec.size() - 1; ++i) {
        accumulator += vec.at(i) + delimiter;
    }
    return accumulator + vec.at(vec.size() - 1);
}
