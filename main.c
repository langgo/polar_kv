#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "io.h"
#include "logstore.h"
#include "hash_map.h"
#include "db.h"

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

        uint64_t l;
        if (-1 == logstore_add_record(logstore, logrecord, &l)) {
            printf("%s\n", strerror(errno));
            exit(-1);
        }
        printf("l1: %lld\n", l);
    }

    {
        char *buf = "higklmn12345";
        logrecord_t logrecord = {
                .buf = buf,
                .size = strlen(buf),
        };

        uint64_t l;
        if (-1 == logstore_add_record(logstore, logrecord, &l)) {
            printf("%s\n", strerror(errno));
            exit(-1);
        }
        printf("l1: %lld\n", l);

        logrecord_t logrecord1;
        if (-1 == logstore_read_record(logstore, l, &logrecord1)) {
            exitErr(-1);
        }
        printf("logrecord: %d %s\n", logrecord1.size, logrecord1.buf);
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
        char key[] = "12345678a";
        uint64_t val = 123456;

        r = hmap_set(hmap, key, 9, val);
        if (r == -1) {
            exitErr(-1);
        }

        uint64_t nval = 0;
        r = hmap_get(hmap, key, 9, &nval);
        if (r == -1) {
            exitErr(-1);
        }
        printf("hmap_get: %lld\n", nval);
    }

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
            printf("val: %lld\n", val);
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
                printf("2. not equal. i: %d, val: %lld\n", i, val);
                exit(-1);
            }
        }
    }

    hmap_delete(hmap);
}

static void t_db() {
    db_t *db;
    if (k_succ != db_open("/tmp/polar_kv", &db)) {
        exitErr(-1);
    }

    {
        int r = 0;
        db_str_t key = {
                .data = "asdf1234",
                .len = 8,
        };

        db_str_t val = {
                .data = "1234asdf",
                .len = 8,
        };

        r = db_put(db, key, val);
        if (r != k_succ) {
            printf("db_put: %d\n", r);
        }

        db_str_t val1;
        r = db_get(db, key, &val1);
        if (r != k_succ) {
            printf("db_get: %d\n", r);
        }
        printf("db_get. val: %ld %s\n", val1.len, val1.data);
    }

    if (k_succ != db_close(db)) {
        exitErr(-1);
    }
}

void t_db_reload() {
    db_t *db;
    if (k_succ != db_open("/tmp/polar_kv", &db)) {
        exitErr(-1);
    }

    {
        int r = 0;
        db_str_t key = {
                .data = "asdf1234x",
                .len = 9,
        };
        db_str_t val = {
                .data = "1234asdf",
                .len = 8,
        };

//        r = db_put(db, key, val);
//        if (r != k_succ) {
//            printf("err. db_put: %d\n", r);
//        }

        db_str_t val1;
        r = db_get(db, key, &val1);
        if (r != k_succ) {
            printf("err. db_get: %d\n", r);
        } else {
            printf("db_get. val: %ld %s\n", val1.len, val1.data);
        }
    }

    if (k_succ != db_close(db)) {
        exitErr(-1);
    }
}

int main() {
    t_db_reload();
    printf("%s\n", "hello");
}
