#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "logstore.h"
#include "coding.h"

const char *wal_fname = "wal.data";

const int writerBufferSize = 65535; // 64KB

const int readBufferCache = 8 * 1024; // 8KB

// read cache

int logstore_new(char *dir, logstore_t **p_logstore) {
    int dirfd = open(dir, O_RDONLY | O_DIRECTORY);
    if (dirfd == -1) {
        return -1;
    }
    int fd = openat(dirfd, wal_fname, O_CREAT | O_RDWR | O_APPEND, 0644);
    if (fd == -1) {
        return -1;
    }
    if (-1 == close(dirfd)) {
        return -1;
    }

    struct stat statbuf = {};
    if (-1 == fstat(fd, &statbuf)) {
        return -1;
    }

    writer_t *writer = NULL;
    if (-1 == writer_new_size(fd, writerBufferSize, &writer)) {
        return -1;
    }

    logstore_t *logstore = (logstore_t *) malloc(sizeof(logstore_t));
    logstore->fd = fd;
    logstore->writer = writer;
    logstore->next_location = statbuf.st_size;

    *p_logstore = logstore;
    return 0;
}

int logstore_delete(logstore_t *logstore) {
    if (-1 == writer_delete(logstore->writer)) {
        return -1;
    }
    if (-1 == close(logstore->fd)) {
        return -1;
    }
    free(logstore);
    return 0;
}

int logstore_add_record(logstore_t *logstore, logrecord_t logrecord, size_t *p_location) {
    {
        writer_t *writer = logstore->writer;

        char buf[4];
        encode_fixed32(buf, (uint32_t) logrecord.size);
        if (-1 == writer_write(writer, buf, 4)) {
            return -1;
        }
        if (-1 == writer_write(writer, logrecord.buf, logrecord.size)) {
            return -1;
        }
        if (-1 == writer_flush(writer)) {
            return -1;
        }
    }

    /*
    {
        int wfd = logstore->fd;

        char buf[4];
        encode_fixed32(buf, (uint32_t) logrecord.size);
        if (-1 == write(wfd, buf, 4)) {
            return -1;
        }
        if (-1 == write(wfd, logrecord.buf, logrecord.size)) {
            return -1;
        }
    }*/
    *p_location = logstore->next_location;
    logstore->next_location += 4 + logrecord.size;
    return 0;
}
