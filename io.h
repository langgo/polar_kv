#ifndef POLAR_KV_IO_H
#define POLAR_KV_IO_H

#include <stddef.h>

typedef struct {
    int fd;
    int size;
    char *buf;
    int len;
} writer_t;

int writer_new(int fd, writer_t **p_writer);

int writer_new_size(int fd, int size, writer_t **p_writer);

int writer_delete(writer_t *writer);

int writer_write(writer_t *writer, char *buf, int size);

int writer_flush(writer_t *writer);

#endif //POLAR_KV_IO_H
