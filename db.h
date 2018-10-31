#ifndef POLAR_KV_DB_H
#define POLAR_KV_DB_H

#include <unistd.h>

typedef struct {

} db_t;

typedef struct {
    char *data;
    size_t len;
} db_str_t;

typedef  struct {

} visitor_t;

typedef enum  {
    e_succ = 0,
    e_not_found = 1,
    e_corruption = 2,
    e_not_supported = 3,
    e_invalid_argument = 4,
    e_io_error = 5,
    e_incomplete = 6,
    e_timedOut = 7,
    e_full = 8,
    e_out_of_memory = 9,
} ret_code_t;

ret_code_t db_open(char *fname, db_t *db);

ret_code_t db_close(db_t *db);

ret_code_t db_put(db_t *db, db_str_t key, db_str_t val);

ret_code_t db_get(db_t *db, db_str_t key, db_str_t *val);

ret_code_t db_range(db_t *db, db_str_t lower, db_str_t ipper, visitor_t *visitor);

#endif //POLAR_KV_DB_H
