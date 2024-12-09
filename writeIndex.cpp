#include <cstdint>

// Functions for big-endian conversion
bool is_bigendian() {
    int x = 1;
    return ((uint8_t *)&x)[0] != 1;
}

uint64_t reverse_bytes(uint64_t x) {
    uint8_t dest[sizeof(uint64_t)];
    uint8_t *source = (uint8_t*)&x;
    for (int c = 0; c < (int)sizeof(uint64_t); c++)
        dest[c] = source[sizeof(uint64_t)-c-1];
    return *(uint64_t*)dest;
}

uint64_t hostToBig(uint64_t x) {
    if (!is_bigendian()) {
        return reverse_bytes(x);
    }
    return x;
}

uint64_t bigToHost(uint64_t x) {
    if (!is_bigendian()) {
        return reverse_bytes(x);
    }
    return x;
}
