#ifndef POLAR_KV_LOGSTORE_H
#define POLAR_KV_LOGSTORE_H

#ifdef __cplusplus
extern "C" {
#endif

// TODO 考虑按块写入

#include "io.h"

typedef struct {
    int fd; // for append record and random read
    writer_t *writer;
    uint64_t next_location;
    uint8_t *readbuf;
} logstore_t;

typedef struct {
    int32_t size;
    char *buf;
} logrecord_t;

typedef struct {
    logstore_t *logstore;
    uint64_t offset;

    size_t buf_size;
    size_t buf_len;
    size_t buf_off; // 表示 buf[off:len] 为有效数据
    char *buf;
} logstore_iter_t;

int logstore_new(char *dir, logstore_t **p_logstore);

int logstore_delete(logstore_t *logstore);

int logstore_add_record(logstore_t *logstore, logrecord_t logrecord, uint64_t *p_location);

int logstore_read_record(logstore_t *logstore, uint64_t location, logrecord_t *logrecord);

int logstore_iter_new(logstore_t *logstore, size_t buf_size, logstore_iter_t **p_iter);

void logstore_iter_delete(logstore_iter_t *iter);

int logstore_iter_next(logstore_iter_t *iter, logrecord_t *p_record, uint64_t *p_location);

#ifdef __cplusplus
}
#endif

#endif //POLAR_KV_LOGSTORE_H
