#include "sha256.hpp"
#include <cstring>
#include <sstream>
#include <iomanip>

#define SHA2_SHFR(x, n)    (x >> n)
#define SHA2_ROTR(x, n)   ((x >> n) | (x << ((sizeof(x) << 3) - n)))
#define SHA2_ROTL(x, n)   ((x << n) | (x >> ((sizeof(x) << 3) - n)))
#define SHA2_CH(x, y, z)  ((x & y) ^ (~x & z))
#define SHA2_MAJ(x, y, z) ((x & y) ^ (x & z) ^ (y & z))
#define SHA256_F1(x) (SHA2_ROTR(x,  2) ^ SHA2_ROTR(x, 13) ^ SHA2_ROTR(x, 22))
#define SHA256_F2(x) (SHA2_ROTR(x,  6) ^ SHA2_ROTR(x, 11) ^ SHA2_ROTR(x, 25))
#define SHA256_F3(x) (SHA2_ROTR(x,  7) ^ SHA2_ROTR(x, 18) ^ SHA2_SHFR(x,  3))
#define SHA256_F4(x) (SHA2_ROTR(x, 17) ^ SHA2_ROTR(x, 19) ^ SHA2_SHFR(x, 10))
#define SHA2_UNPACK32(x, str)                 \
{                                             \
    *((str) + 3) = (uint8_t) ((x)      );       \
    *((str) + 2) = (uint8_t) ((x) >>  8);       \
    *((str) + 1) = (uint8_t) ((x) >> 16);       \
    *((str) + 0) = (uint8_t) ((x) >> 24);       \
}
#define SHA2_PACK32(str, x)                   \
{                                             \
    *(x) =   ((uint32_t) *((str) + 3)      )    \
           | ((uint32_t) *((str) + 2) <<  8)    \
           | ((uint32_t) *((str) + 1) << 16)    \
           | ((uint32_t) *((str) + 0) << 24);   \
}

SHA256::SHA256() {
    state[0] = 0x6a09e667;
    state[1] = 0xbb67ae85;
    state[2] = 0x3c6ef372;
    state[3] = 0xa54ff53a;
    state[4] = 0x510e527f;
    state[5] = 0x9b05688c;
    state[6] = 0x1f83d9ab;
    state[7] = 0x5be0cd19;
    count[0] = 0;
    count[1] = 0;
}

void SHA256::transform(const uint8_t* data) {
    uint32_t message[16], w[64];
    uint32_t t1, t2;
    uint32_t a, b, c, d, e, f, g, h;
    int i, j;

    for (i = 0, j = 0; i < 16; i++, j += 4)
        SHA2_PACK32(data + j, &message[i]);

    for (i = 0; i < 16; i++) w[i] = message[i];
    for (i = 16; i < 64; i++)
        w[i] = SHA256_F4(w[i - 2]) + w[i - 7] + SHA256_F3(w[i - 15]) + w[i - 16];

    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];
    f = state[5];
    g = state[6];
    h = state[7];

    static const uint32_t k[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
        0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
        0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
        0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
        0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
        0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
        0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
        0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
        0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    for (i = 0; i < 64; i++) {
        t1 = h + SHA256_F2(e) + SHA2_CH(e, f, g) + k[i] + w[i];
        t2 = SHA256_F1(a) + SHA2_MAJ(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    state[5] += f;
    state[6] += g;
    state[7] += h;
}

void SHA256::update(const uint8_t* data, size_t length) {
    uint32_t i, j;

    j = (count[0] >> 3) & 63;
    if ((count[0] += (uint32_t) (length << 3)) < (length << 3))
        count[1]++;
    count[1] += (uint32_t) (length >> 29);

    if ((j + length) > 63) {
        std::memcpy(&buffer[j], data, (i = 64 - j));
        transform(buffer);
        for (; i + 63 < length; i += 64)
            transform(&data[i]);
        j = 0;
    } else
        i = 0;

    std::memcpy(&buffer[j], &data[i], length - i);
}

void SHA256::update(const std::string& data) {
    update((const uint8_t*) data.data(), data.size());
}

std::vector<uint8_t> SHA256::digest() {
    uint32_t i, j;
    uint8_t final_count[8];
    uint8_t digest[32];

    SHA2_UNPACK32(count[1], final_count);
    SHA2_UNPACK32(count[0], final_count + 4);

    update((uint8_t*) "\200", 1);
    while ((count[0] & 504) != 448)
        update((uint8_t*) "\0", 1);

    update(final_count, 8);

    for (i = 0; i < 8; i++)
        SHA2_UNPACK32(state[i], &digest[i << 2]);
    
    return std::vector<uint8_t>(digest, digest + 32);
}

std::string SHA256::hash(const std::string& data) {
    SHA256 sha;
    sha.update(data);
    std::vector<uint8_t> digest = sha.digest();
    
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (uint8_t b : digest) {
        ss << std::setw(2) << (int)b;
    }
    return ss.str();
}

std::vector<uint8_t> SHA256::hashBytes(const std::vector<uint8_t>& data) {
    SHA256 sha;
    sha.update(data.data(), data.size());
    return sha.digest();
}
