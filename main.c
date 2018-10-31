#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "io.h"
#include "logstore.h"

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

void t_logstore() {
    logstore_t *logstore;
    if (-1 == logstore_new("/tmp/polar_kv", &logstore)) {
        printf("%s\n", strerror(errno));
        exit(-1);
    }

    {
        char *buf = "abcdefg12345";
        logrecord_t logrecord = {
                .buf = buf,
                .size = strlen(buf),
        };

        size_t l;
        if (-1 == logstore_add_record(logstore, logrecord, &l)) {
            printf("%s\n", strerror(errno));
            exit(-1);
        }
        printf("l1: %ld\n", l);
    }

    {
        char *buf = "higklmn12345";
        logrecord_t logrecord = {
                .buf = buf,
                .size = strlen(buf),
        };

        size_t l;
        if (-1 == logstore_add_record(logstore, logrecord, &l)) {
            printf("%s\n", strerror(errno));
            exit(-1);
        }
        printf("l1: %ld\n", l);
    }

    if (-1 == logstore_delete(logstore)) {
        printf("%s\n", strerror(errno));
        exit(-1);
    }
}

int main() {
    printf("%s\n", "hello");
    t_logstore();
}