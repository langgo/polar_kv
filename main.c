#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "io.h"

void t_writer() {
    int fd = open("/tmp/test_writer.txt", O_CREAT | O_TRUNC | O_RDWR, 0644);
    if (fd == -1) {
        printf("%s\n", strerror(errno));
        exit(-1);
    }

    writer_t *writer = NULL;
    if (-1 == writer_new(fd, &writer)) {
        printf("%s\n", strerror(errno));
        exit(-1);
    }

    char *buf = "1234567890";
    if (-1 == writer_write(writer, buf, strlen(buf))) {
        printf("%s\n", strerror(errno));
        exit(-1);
    }

    printf("writer->buf: %d, %s\n", writer->len, writer->buf);

    if (-1 == writer_flush(writer)) {
        printf("%s\n", strerror(errno));
        exit(-1);
    }

    printf("writer->buf: %d, %s\n", writer->len, writer->buf);

    close(fd);
}


int main() {
    printf("%s\n", "hello");
    t_writer();
}