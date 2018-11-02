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

            /*
            for (int i = 0; i < HMAP_ITEM_SIZE; ++i) {
                if (item->tops[i] >= minTopHash) {
                    int _len = (uint16_t) (item->vals[i] >> 48);
                    if (_len > 8) {
                        free((char *) item->keys[i]);
                    }
                }
            }
             */

            free(item);
            item = next;
        }
    }
    free(hmap->items);
    free(hmap);
}

static int hmap_g(hmap_t *hmap, char *key, int key_len, uint64_t value,
                  int *p_index, uint8_t *p_top, uint64_t *p_nkey, uint64_t *p_nval) {

    uint64_t hash = siphash(key, key_len, seed);
    int index = (int) (hash & hmap->sizemask);
    uint8_t top = (uint8_t) (hash >> (sizeof(uint64_t) - 8));
    if (top < minTopHash) {
        top += minTopHash;
    }

    uint64_t a = 1;
    assert(key_len < ((a << 16) - 1));
    assert(value < ((a << 48) - 1));

    uint64_t nkey = 0;
    if (key_len <= 8) {
        for (int i = 0; i < key_len; ++i) {
            uint64_t tmp = (uint64_t) key[i];
            nkey = nkey | (tmp << 8 * i);
        }
    } else {
        char *key1 = malloc(sizeof(char) * key_len);
        if (key1 == NULL) {
            return -1;
        }
        memcpy(key1, key, key_len);
        nkey = (uint64_t) key1;
    }

    *p_index = index;
    *p_top = top;
    *p_nkey = nkey;
    if (p_nval != NULL) {
        *p_nval = (uint64_t) (key_len) << 48 | value; // 16 + 48;
    }
    return 0;
}

int hmap_set(hmap_t *hmap, char *key, int key_len, uint64_t value) {
    int index;
    uint8_t top;
    uint64_t nkey;
    uint64_t nval;
    hmap_g(hmap, key, key_len, value, &index, &top, &nkey, &nval);

    hmap_item_t *prev = NULL;
    hmap_item_t *item = hmap->items[index];
    while (item != NULL) {
        for (int i = 0; i < HMAP_ITEM_SIZE; ++i) {
            int _len = (uint16_t) (item->vals[i] >> 48);
            if (item->tops[i] == top && _len == key_len) {
                if (_len <= 8 && item->keys[i] == nkey || memcmp((char *) item->keys[i], (char *) nkey, _len) == 0) {
                    item->vals[i] = nval;
                    return 0;
                }
            } else if (item->tops[i] == topEmpty) {
                item->tops[i] = top;
                item->keys[i] = nkey;
                item->vals[i] = nval;
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
    nitem->keys[0] = nkey;
    nitem->vals[0] = nval;

    return 0;
}

// -2 not found
int hmap_get(hmap_t *hmap, char *key, int key_len, uint64_t *p_value) {
    int index;
    uint8_t top;
    uint64_t nkey;
    hmap_g(hmap, key, key_len, 0, &index, &top, &nkey, NULL);

    hmap_item_t *item = hmap->items[index];
    while (item != NULL) {
        for (int i = 0; i < HMAP_ITEM_SIZE; ++i) {
            int _len = (uint16_t) (item->vals[i] >> 48);
            if (item->tops[i] == top && _len == key_len) {
                if (_len <= 8 && item->keys[i] == nkey || memcmp((char *) item->keys[i], (char *) nkey, _len) == 0) {
                    uint64_t tmp = item->vals[i];
                    *p_value = tmp << 16 >> 16;
                    return 0;
                }
            }
        }
        item = item->next;
    }
    return -2;
}

// TODO 这里的key怎么释放。由用户决定。意思是，它需要遍历，然后释放内存。

