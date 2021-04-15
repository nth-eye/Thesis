#ifndef HOTP_H
#define HOTP_H

#include <cstddef>
#include <cstdint>
#include <cstring>

constexpr int ipow(int base, int exp)
{
    int result = 1;
    while (1) {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        if (!exp)
            break;
        base *= base;
    }
    return result;
}

template<class H>
constexpr void hmac(H &hash, uint8_t *msg, size_t msg_len, uint8_t *key, size_t key_len, uint8_t *digest)
{
    uint8_t k_ipad[H::BLOCK_SIZE] = {};
    uint8_t k_opad[H::BLOCK_SIZE] = {};
    uint8_t tk[H::HASH_SIZE] = {};

    if (key_len > H::BLOCK_SIZE) {

        hash.init();
        hash.update(key, key_len);
        hash.final(tk);

        key = tk;
        key_len = H::HASH_SIZE;
    }

    memcpy(k_ipad, key, key_len);
    memcpy(k_opad, key, key_len);

    for (size_t i = 0; i < H::BLOCK_SIZE; ++i) {
        k_ipad[i] ^= 0x36;
        k_opad[i] ^= 0x5C;
    }

    hash.init();
    hash.update(k_ipad, H::BLOCK_SIZE);
    hash.update(msg, msg_len);
    hash.final(digest);

    hash.init();
    hash.update(k_opad, H::BLOCK_SIZE);
    hash.update(digest, H::HASH_SIZE);
    hash.final(digest);
}

template<class H>
constexpr uint32_t hotp(H &hash, uint8_t *key, size_t key_len, uint64_t count)
{
    static_assert(H::HASH_SIZE >= 20, "HOTP hash algorithm must provide at least 20 byte long value");

    uint8_t digest[H::HASH_SIZE] = {};

#if 'ABCD' == 0x41424344
    count = (count & 0x00000000ffffffff) << 32 | (count & 0xffffffff00000000) >> 32;
    count = (count & 0x0000ffff0000ffff) << 16 | (count & 0xffff0000ffff0000) >> 16;
    count = (count & 0x00ff00ff00ff00ff) <<  8 | (count & 0xff00ff00ff00ff00) >>  8;
#endif
    hmac(hash, (uint8_t *) &count, sizeof(count), key, key_len, digest);

    uint32_t offset = digest[H::HASH_SIZE - 1] & 0x0f;
    uint32_t bin_code = 
            (digest[offset]/*& 0x7f*/)  << 24 |
            (digest[offset + 1])        << 16 |
            (digest[offset + 2])        <<  8 |
            (digest[offset + 3]);

    return bin_code; // % ipow(10, 6);
}

#endif // HOTP_H