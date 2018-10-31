#include <string.h>
#include "coding.h"

void encode_fixed32(char *buf, uint32_t value) {
    memcpy(buf, &value, sizeof(value));
}

inline uint32_t decode_fixed32(const char *buf) {
    uint32_t result;
    memcpy(&result, buf, sizeof(result));
    return result;
}
