#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include "io.h"

const int defaultBufferSize = 4096;

int writer_new(int fd, writer_t **p_writer) {
    return writer_new_size(fd, defaultBufferSize, p_writer);
}

int writer_new_size(int fd, int size, writer_t **p_writer) {
    char *buf = malloc(sizeof(char) * size);
    if (buf == NULL) {
        return -1;
    }

    writer_t *writer = (writer_t *) malloc(sizeof(writer_t));
    if (writer == NULL) {
        return -1;
    }

    writer->fd = fd;
    writer->buf = buf;
    writer->size = size;
    writer->len = 0;

    *p_writer = writer;
    return 0;
}

int writer_delete(writer_t *writer) {
    if (-1 == writer_flush(writer)) {
        return -1;
    }
    free(writer->buf);
    free(writer);
    return 0;
}

inline int min(int a, int b) {
    if (a < b) {
        return a;
    }
    return b;
}

int writer_available(writer_t *writer) {
    return writer->size - writer->len;
}

int writer_write(writer_t *writer, char *buf, int size) {
    int nn = 0;
    while (size > writer_available(writer)) {
        ssize_t n = 0;
        if (writer_available(writer) == 0) {
            // Large write, empty buffer.
            // Write directly from buf to avoid copy.
            n = write(writer->fd, buf, size);
            if (n == -1) {
                return -1;
            }
        } else {
            assert(size > writer_available(writer));
            n = writer_available(writer);
            memcpy(writer->buf + writer->len, buf, n);
            writer->len += n;
            if (-1 == writer_flush(writer)) {
                return -1;
            }
        }
        nn += n;
        buf += n;
        size -= n;
    }
    assert(size < writer_available(writer));
    memcpy(writer->buf + writer->len, buf, size);
    writer->len += size;
    nn += size;
    return nn;
}

int writer_flush(writer_t *writer) {
    int len = writer->len;
    while (len) {
        int n = write(writer->fd, writer->buf, len);
        if (n == -1) {
            return -1;
        }
        len -= n;
    }
    writer->len = 0;
    return 0;
}