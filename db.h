#ifndef POLAR_KV_DB_H
#define POLAR_KV_DB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <stdint.h>
#include "logstore.h"
#include "hash_map.h"

typedef struct {
    logstore_t *logstore;
    hmap_t *hmap;
    uint64_t count;
} db_t;

typedef struct {
    char *data;
    size_t len;
} db_str_t;

typedef struct {

} visitor_t;

typedef enum {
    k_succ = 0,
    k_not_found = 1,
    k_corruption = 2,
    k_not_supported = 3,
    k_invalid_argument = 4,
    k_io_error = 5,
    k_incomplete = 6,
    k_timedOut = 7,
    k_full = 8,
    k_out_of_memory = 9,
    k_all = 10,
} ret_code_t;

ret_code_t db_open(char *fname, db_t **p_db);

ret_code_t db_close(db_t *db);

ret_code_t db_put(db_t *db, db_str_t key, db_str_t val);

ret_code_t db_get(db_t *db, db_str_t key, db_str_t *val);

ret_code_t db_range(db_t *db, db_str_t lower, db_str_t ipper, visitor_t *visitor);

#ifdef __cplusplus
}
#endif

#endif //POLAR_KV_DB_H
