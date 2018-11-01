#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "io.h"
#include "logstore.h"
#include "hash_map.h"

void exitErr(int code) {
    printf("%s\n", strerror(errno));
    exit(code);
}

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

void t_hmap() {
    hmap_t *hmap;
    if (-1 == hmap_new(8, &hmap)) {
        exitErr(-1);
    }
/*
    {
        uint64_t key = 1234;
        uint64_t val = 0;
        int r = hmap_get(hmap, (char *) (&key), 8, &val);
        if (r == -1) {
            exitErr(-1);
        }
        printf("hmap_get: %d\n", r);
    }

    {
        int r = 0;
        uint64_t key = 1234;
        uint64_t val = 123456;

        r = hmap_set(hmap, (char *) (&key), 8, val);
        if (r == -1) {
            exitErr(-1);
        }

        uint64_t nval = 0;
        r = hmap_get(hmap, (char *) (&key), 8, &nval);
        if (r == -1) {
            exitErr(-1);
        }
        printf("hmap_get: %d\n", nval);
    }*/

    {
        int r = 0;
        for (int i = 0; i < 10; ++i) {
            uint64_t key = i;
            uint64_t val = i * 2;

            r = hmap_set(hmap, (char *) (&key), 8, val);
            if (r == -1) {
                exitErr(-1);
            }

            val = 0;
            r = hmap_get(hmap, (char *) (&key), 8, &val);
            if (r == -1) {
                exitErr(-1);
            }
            if (r == -2) {
                printf("1. not found. i: %d\n", i);
                exit(-1);
            }
            if (val != i * 2) {
                printf("1. not equal. i: %d\n", i);
                exit(-1);
            }
            printf("val: %d\n", val);
        }

        for (int i = 0; i < 10; ++i) {
            uint64_t key = i;
            uint64_t val = 0;

            r = hmap_get(hmap, (char *) (&key), 8, &val);
            if (r == -1) {
                exitErr(-1);
            }
            if (r == -2) {
                printf("2. not found. i: %d\n", i);
                exit(-1);
            }
            if (val != i * 2) {
                printf("2. not equal. i: %d, val: %ld\n", i, val);
                exit(-1);
            }
        }
    }

    hmap_delete(hmap);
}

int main() {
    t_hmap();
    printf("%s\n", "hello");
}
