#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "db.h"
#include "logstore.h"
#include "coding.h"

ret_code_t db_init(db_t *db) {
    logstore_iter_t *iter;
    if (-1 == logstore_iter_new(db->logstore, 1024 * 1024, &iter)) {
        return k_out_of_memory;
    }

    while (1) {
        logrecord_t logrecord;
        uint64_t location;

        int r = logstore_iter_next(iter, &logrecord, &location);
        if (-1 == r) {
            return k_io_error;
        }
        if (-2 == r) {
            break;
        }

        uint16_t key_len = (uint16_t) decode_fixed32(logrecord.buf);

        if (-1 == hmap_set(db->hmap, logrecord.buf + 4, key_len, location)) {
            return k_out_of_memory;
        }
        db->count++;
    }

    logstore_iter_delete(iter);

    return k_succ;
}

ret_code_t db_open(char *dir, db_t **p_db) {
    hmap_t *hmap;
    if (-1 == hmap_new(4096, &hmap)) {
        return k_out_of_memory;
    }
    logstore_t *logstore;
    if (-1 == logstore_new(dir, &logstore)) {
        return k_io_error;
    }

    db_t *db = malloc(sizeof(db_t));
    if (db == NULL) {
        return k_out_of_memory;
    }
    db->logstore = logstore;
    db->hmap = hmap;
    db->count = 0;

    ret_code_t r = db_init(db);
    if (r != k_succ) {
        return r;
    }

    *p_db = db;
    return k_succ;
}

ret_code_t db_close(db_t *db) {
    hmap_delete(db->hmap);
    if (-1 == logstore_delete(db->logstore)) {
        return k_all;
    }
    free(db);
    return k_succ;
}

ret_code_t db_put(db_t *db, db_str_t key, db_str_t val) {
    assert(key.len < ((1 << 16) - 1));
    assert(val.len < ((1 << 20) - 4));

    // TODO 避免复制

    logrecord_t logrecord;
    logrecord.buf = malloc(sizeof(char) * (4 + key.len + 4 + val.len));
    if (logrecord.buf == NULL) {
        return k_out_of_memory;
    }

    int bufix = 0;
    encode_fixed32(logrecord.buf + bufix, (uint32_t) key.len);
    bufix += 4;
    memcpy(logrecord.buf + bufix, key.data, key.len);
    bufix += key.len;
    encode_fixed32(logrecord.buf + bufix, (uint32_t) val.len);
    bufix += 4;
    memcpy(logrecord.buf + bufix, val.data, val.len);
    bufix += val.len;

    logrecord.size = bufix;

    uint64_t location = 0;
    if (-1 == logstore_add_record(db->logstore, logrecord, &location)) {
        return k_all;
    }

    free(logrecord.buf);
    if (-1 == hmap_set(db->hmap, key.data, key.len, location)) {
        return k_all;
    }
    db->count++;
    return k_succ;
}

ret_code_t db_get(db_t *db, db_str_t key, db_str_t *val) {
    uint64_t location = 0;
    int r = hmap_get(db->hmap, key.data, key.len, &location);
    if (r == -1) {
        return k_all;
    }
    if (r == -2) {
        return k_not_found;
    }

    logrecord_t logrecord;
    r = logstore_read_record(db->logstore, location, &logrecord);
    if (r == -1) {
        return k_all;
    }

    int key_len = decode_fixed32(logrecord.buf);
    val->len = decode_fixed32(logrecord.buf + 4 + key_len);

    val->data = malloc(sizeof(char) * val->len);
    if (val->data == NULL) {
        return k_out_of_memory;
    }
    memcpy(val->data, logrecord.buf + 4 + key_len + 4, val->len);
    free(logrecord.buf);

    return k_succ;
}
