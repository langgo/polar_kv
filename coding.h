#ifndef POLAR_KV_CODING_H
#define POLAR_KV_CODING_H

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>

void encode_fixed32(char *buf, uint32_t value);

uint32_t decode_fixed32(const char *buf);

#ifdef __cplusplus
}
#endif

#endif //POLAR_KV_CODING_H
