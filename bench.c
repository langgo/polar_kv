#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/time.h>
#include "io.h"
#include "logstore.h"
#include "hash_map.h"
#include "db.h"

void exitErr(int code) {
    printf("-- %s\n", strerror(errno));
    exit(code);
}

// need free
char *randString(size_t size) {
    char *str = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    size_t len = strlen(str);

    char *ret = malloc(size + 1);
    for (int i = 0; i < size; ++i) {
        ret[i] = str[random() % len];
    }
    ret[size] = '\0';
    return ret;
}

double delta(struct timeval t1, struct timeval t2) {
    return ((double) t2.tv_sec + (double) t2.tv_usec / 1000000) - ((double) t1.tv_sec + (double) t1.tv_usec / 1000000);
}

typedef struct {
    db_str_t key;
    db_str_t val;
} pair_t;

static void t_db() {
    // 1. OPEN
    db_t *db;
    {
        struct timeval t1, t2;
        gettimeofday(&t1, NULL);

        if (k_succ != db_open("/tmp/polar_kv", &db)) {
            exitErr(-1);
        }

        gettimeofday(&t2, NULL);
        double d = delta(t1, t2);
        printf("OPEN time: %lf s, %lf w/s\n", d, db->count / d / 10000);
    }

    // 2. 构造测试数据 1w 条
    const int count = 10 * 10000;
    pair_t *pairs = malloc(sizeof(pair_t) * count);

    {
        struct timeval t1, t2;
        gettimeofday(&t1, NULL);

        for (int i = 0; i < count; ++i) {
            if (i % 100 == 1) {
                pairs[i].key.len = 16;
                pairs[i].key.data = randString(16);
            } else {
                pairs[i].key.len = 8;
                pairs[i].key.data = randString(8);
            }

            pairs[i].val.len = 4 * 1024;
            pairs[i].val.data = randString(4 * 1024);
        }

        gettimeofday(&t2, NULL);
        double d = delta(t1, t2);
        printf("DATA time: %lf s, %lf w/s\n", d, count / d / 10000);
    }

    // 3. 测试过程
    if (1) { // write
        struct timeval t1, t2;
        gettimeofday(&t1, NULL);

        for (int i = 0; i < count; ++i) {
            if (k_succ != db_put(db, pairs[i].key, pairs[i].val)) {
                exitErr(-1);
            }
        }

        gettimeofday(&t2, NULL);
        double d = delta(t1, t2);
        printf("PUT time: %lf s, %lf w/s\n", d, count / d / 10000);
    }

    if (1) { // read
        struct timeval t1, t2;
        gettimeofday(&t1, NULL);

        for (int i = 0; i < count; ++i) {
            db_str_t val;
            if (k_succ != db_get(db, pairs[i].key, &val)) {
                exitErr(-1);
            }

            if (0 != memcmp(pairs[i].val.data, val.data, 4 * 1024)) {
                printf("not equal. i: %d, key: %s,", i, pairs[i].key.data);
                exit(-1);
            }
        }

        gettimeofday(&t2, NULL);
        double d = delta(t1, t2);
        printf("GET time: %lf s, %lf w/s\n", d, count / d / 10000);
    }

    if (k_succ != db_close(db)) {
        exitErr(-1);
    }
}

int main() {
    t_db();
    printf("%s\n", "hello");
}
