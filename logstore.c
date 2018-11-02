#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
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

    // TODO
    uint8_t *readbuf = malloc(sizeof(uint8_t) * 8 * 1024);
    if (readbuf == NULL) {
        return -1;
    }

    logstore_t *logstore = (logstore_t *) malloc(sizeof(logstore_t));
    logstore->fd = fd;
    logstore->writer = writer;
    logstore->next_location = statbuf.st_size;
    logstore->readbuf = readbuf;

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

int logstore_add_record(logstore_t *logstore, logrecord_t logrecord, uint64_t *p_location) {
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

int logstore_read_record(logstore_t *logstore, uint64_t location, logrecord_t *logrecord) {
    int rfd = logstore->fd;

    // TODO 如果能够知道value的长度，就不需要多读了。
    int size = readBufferCache;
    while (size > 0) {
        ssize_t n = pread(rfd, logstore->readbuf, size, location);
        if (n == -1) {
            return -1;
        }
        if (n == 0) {
            break;
        }
        size -= n;
        location += n;
    }

    // 解析 record 的长度

    uint32_t length = decode_fixed32((const char *) logstore->readbuf);

    char *content = malloc(sizeof(char) * length);
    if (content == NULL) {
        return -1;
    }

    int bufrealsize = length > readBufferCache - 4 ? readBufferCache - 4 : length;
    memcpy(content, logstore->readbuf + 4, bufrealsize);

    int remain = length - bufrealsize;
    while (remain > 0) {
        ssize_t n = pread(rfd, content + bufrealsize, remain, location);
        if (n == -1) {
            return -1;
        }
        if (n == 0) {
            break;
        }
        remain -= n;
        location += n;
    }
    assert(remain == 0);

    logrecord->size = length;
    logrecord->buf = content;

    return 0;
}

// Note: 这里假设 record 的最大长度小雨 buf_size-4
int logstore_iter_new(logstore_t *logstore, size_t buf_size, logstore_iter_t **p_iter) {
    char *buf = malloc(sizeof(char) * buf_size);
    if (buf == NULL) {
        return -1;
    }
    logstore_iter_t *iter = malloc(sizeof(logstore_iter_t));
    if (iter == NULL) {
        return -1;
    }

    iter->logstore = logstore;
    iter->offset = 0;
    iter->buf_size = buf_size;
    iter->buf_len = 0;
    iter->buf_off = 0;
    iter->buf = buf;

    *p_iter = iter;
    return 0;
}

void logstore_iter_delete(logstore_iter_t *iter) {
    free(iter->buf);
    free(iter);
}


int _logstore_iter_reread(logstore_iter_t *iter) {
    iter->buf_len = 0;
    iter->buf_off = 0;

    size_t size = iter->buf_size;
    while (size > 0) {
        ssize_t n = pread(iter->logstore->fd, iter->buf + iter->buf_len, size, iter->offset);
        if (n == -1) {
            return -1;
        }
        if (n == 0) {
            break;
        }
        size -= n;
        iter->buf_len += n;
        iter->offset += n;
    }
    return 0;
}

// -2 EOF
int logstore_iter_next(logstore_iter_t *iter, logrecord_t *p_record, uint64_t *p_location) {
    if (iter->buf_len - iter->buf_off < 4) {
        if (-1 == _logstore_iter_reread(iter)) {
            return -1;
        }

        if (iter->buf_len == 0) {
            return -2;
        }
    }

    uint32_t len = decode_fixed32(iter->buf + iter->buf_off);
    if (iter->buf_len - iter->buf_off - 4 < len) {
        iter->offset -= iter->buf_len - iter->buf_off;
        _logstore_iter_reread(iter);
    }

    p_record->size = len;
    p_record->buf = iter->buf + iter->buf_off + 4; // Note: 避免copy

    *p_location = iter->offset - (iter->buf_len - iter->buf_off);
    iter->buf_off += len + 4;
    return 0;
}

