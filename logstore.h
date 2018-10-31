#ifndef POLAR_KV_LOGSTORE_H
#define POLAR_KV_LOGSTORE_H

// TODO 考虑按块写入

#include "io.h"

typedef struct {
    int fd; // for append record and random read
    writer_t *writer;
    size_t next_location;
} logstore_t;

typedef struct {
    int32_t size;
    char *buf;
} logrecord_t;

int logstore_new(char *dir, logstore_t **p_logstore);

int logstore_delete(logstore_t *logstore);

int logstore_add_record(logstore_t *logstore, logrecord_t logrecord, size_t *p_location);

#endif //POLAR_KV_LOGSTORE_H
