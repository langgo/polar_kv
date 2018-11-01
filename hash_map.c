#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "hash_map.h"

uint64_t siphash(const uint8_t *in, const size_t inlen, const uint8_t *k);

const uint8_t seed[17] = "0123456789abcdef";

const uint8_t topEmpty = 0;
const uint8_t minTopHash = 1;

uint64_t _next_power(int size) {
    uint64_t i = 4; // TODO
    while (1) {
        if (i >= size) {
            return i;
        }
        i *= 2;
    }
}

int hmap_new(uint64_t size, hmap_t **p_hmap) {
    size = _next_power(size);

    hmap_t *hmap = malloc(sizeof(hmap_t));
    if (hmap == NULL) {
        return -1;
    }
    hmap_item_t **items = malloc(sizeof(hmap_item_t *) * size);
    if (items == NULL) {
        return -1;
    }

    memset(items, 0, sizeof(hmap_item_t *) * size);
    hmap->size = size;
    hmap->sizemask = size - 1;
    hmap->items = items;

    *p_hmap = hmap;
    return 0;
}

void hmap_delete(hmap_t *hmap) {
    for (int i = 0; i < hmap->size; i++) {
        hmap_item_t *item = hmap->items[i];
        while (item != NULL) {
            hmap_item_t *next = item->next;
            free(item);
            item = next;
        }
    }
    free(hmap->items);
    free(hmap);
}

int hmap_set(hmap_t *hmap, char *key, int key_len, uint64_t value) {
    uint64_t hash = siphash(key, key_len, seed);
    int index = (int) (hash & hmap->sizemask);
    uint8_t top = (uint8_t) (hash >> (sizeof(uint64_t) - 8));
    if (top < minTopHash) {
        top += minTopHash;
    }

    hmap_item_t *prev = NULL;
    hmap_item_t *item = hmap->items[index];
    while (item != NULL) {
        for (int i = 0; i < HMAP_ITEM_SIZE; ++i) {
            if (item->tops[i] == top) {
                if (item->keys[i] == *(uint64_t *) (key)) {
                    item->vals[i] = value;
                    return 0;
                }
            } else if (item->tops[i] == topEmpty) {
                assert(key_len == 8);
                item->tops[i] = top;
                item->keys[i] = *(uint64_t *) (key);
                item->vals[i] = value;
                return 0;
            }
        }

        prev = item;
        item = item->next;
    }

    hmap_item_t *nitem = malloc(sizeof(hmap_item_t));
    if (nitem == NULL) {
        return -1;
    }
    memset(nitem, 0, sizeof(hmap_item_t));
    if (prev != NULL) {
        prev->next = nitem;
    } else {
        hmap->items[index] = nitem;
    }
    nitem->tops[0] = top;
    nitem->keys[0] = *(uint64_t *) (key);
    nitem->vals[0] = value;

    return 0;
}

// -2 not found
int hmap_get(hmap_t *hmap, char *key, int key_len, uint64_t *p_value) {
    uint64_t hash = siphash(key, key_len, seed);
    int index = (int) (hash & hmap->sizemask);
    uint8_t top = (uint8_t) (hash >> (sizeof(uint64_t) - 8));
    if (top < minTopHash) {
        top += minTopHash;
    }

    hmap_item_t *item = hmap->items[index];
    while (item != NULL) {
        for (int i = 0; i < HMAP_ITEM_SIZE; ++i) {
            if (item->tops[i] == top) {
                // TODO 这里先假设所有的key都是 8B，如果大于8B，也要把key存储在 内存中，如果内存放不下。考虑第二种方案。
                assert(key_len == 8);
                if (key_len != 8) {
                    printf("key_len != 8, %d\n", key_len);
                }
                if (item->keys[i] == *(uint64_t *) (key)) {
                    *p_value = item->vals[i];
                    return 0;
                }
            }
        }
        item = item->next;
    }
    return -2;
}
