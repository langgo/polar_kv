#ifndef POLAR_KV_HASH_MAP_H
#define POLAR_KV_HASH_MAP_H

#include <unistd.h>

#ifndef HMAP_ITEM_SIZE
#define HMAP_ITEM_SIZE (8)
#endif

typedef struct hmap_item_s {
    uint8_t tops[HMAP_ITEM_SIZE];
    uint64_t keys[HMAP_ITEM_SIZE];
    uint64_t vals[HMAP_ITEM_SIZE];
    struct hmap_item_s *next;
} hmap_item_t;

typedef struct {
    uint64_t size;
    uint64_t sizemask;
    hmap_item_t **items;
} hmap_t;

// ----|----|----|----|----|----|----|----
// top  top  top  top
//

// 一个 value 对应一个 8K 的页的时候，
// 有两个 map 一个是 immap的节点, 一个是 map
// 持久化，immap的节点，并记录持久化日志。

int hmap_new(uint64_t size, hmap_t **p_hmap);

void hmap_delete(hmap_t *hmap);

int hmap_set(hmap_t *hmap, char *key, uint16_t key_len, uint64_t value);

int hmap_get(hmap_t *hmap, char *key, uint16_t key_len, uint64_t *p_value);

#endif //POLAR_KV_HASH_MAP_H
