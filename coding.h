#ifndef POLAR_KV_CODING_H
#define POLAR_KV_CODING_H

#include <unistd.h>

void encode_fixed32(char *buf, uint32_t value);

uint32_t decode_fixed32(const char *buf);

#endif //POLAR_KV_CODING_H
