#include <stdlib.h>
#include <string.h>
#include "db.h"
#include "logstore.h"
#include "coding.h"

ret_code_t db_init() {

}

ret_code_t db_open(char *dir, db_t **p_db) {
    hmap_t *hmap;
    if (-1 == hmap_new(8, &hmap)) {
        return e_out_of_memory;
    }
    logstore_t *logstore;
    if (-1 == logstore_new(dir, &logstore)) {
        return e_io_error;
    }

    db_t *db = malloc(sizeof(db));
    if (db == NULL) {
        return e_out_of_memory;
    }
    db->logstore = logstore;
    db->hmap = hmap;

    *p_db = db;
    return e_succ;
}

ret_code_t db_close(db_t *db) {
    hmap_delete(db->hmap);
    if (-1 == logstore_delete(db->logstore)) {
        return e_all;
    }
    free(db);
    return e_succ;
}

ret_code_t db_put(db_t *db, db_str_t key, db_str_t val) {
    // TODO 避免复制

    logrecord_t logrecord;
    logrecord.buf = malloc(sizeof(char) * (4 + key.len + 4 + val.len));
    if (logrecord.buf == NULL) {
        return e_out_of_memory;
    }

    int bufix = 0;
    encode_fixed32(logrecord.buf + bufix, key.len);
    bufix += 4;
    memcpy(logrecord.buf + bufix, key.data, key.len);
    bufix += key.len;
    encode_fixed32(logrecord.buf + bufix, val.len);
    bufix += 4;
    memcpy(logrecord.buf + bufix, val.data, val.len);
    bufix += val.len;

    logrecord.size = bufix;

    off_t offset = 0;
    if (-1 == logstore_add_record(db->logstore, logrecord, &offset)) {
        return e_all;
    }

    free(logrecord.buf);
    if (-1 == hmap_set(db->hmap, key.data, key.len, offset)) {
        return e_all;
    }
    return e_succ;
}

ret_code_t db_get(db_t *db, db_str_t key, db_str_t *val) {
    off_t offset = 0;
    int r = hmap_get(db->hmap, key.data, key.len, &offset);
    if (r == -1) {
        return e_all;
    }
    if (r == -2) {
        return e_not_found;
    }

    logrecord_t logrecord;
    r = logstore_read_record(db->logstore, offset, &logrecord);
    if (r == -1) {
        return e_all;
    }

    int key_len = decode_fixed32(logrecord.buf);
    val->len = decode_fixed32(logrecord.buf + 4 + key_len);

    val->data = malloc(sizeof(char) * val->len);
    if (val->data == NULL) {
        return e_out_of_memory;
    }
    memcpy(val->data, logrecord.buf + 4 + key_len + 4, val->len);
    free(logrecord.buf);

    return e_not_supported;
}
