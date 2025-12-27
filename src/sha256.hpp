#ifndef SHA256_H
#define SHA256_H

#include <string>
#include <vector>
#include <cstdint>

class SHA256 {
public:
    SHA256();
    void update(const uint8_t* data, size_t length);
    void update(const std::string& data);
    std::vector<uint8_t> digest();
    static std::string hash(const std::string& data);
    static std::vector<uint8_t> hashBytes(const std::vector<uint8_t>& data);

private:
    uint32_t state[8];
    uint32_t count[2];
    uint8_t buffer[64];

    void transform(const uint8_t* data);
};

#endif
