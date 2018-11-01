cmake_minimum_required(VERSION 3.9)
project(polar_kv)

set(CMAKE_CXX_STANDARD 11)

add_executable(main
        main.c
        db.c db.h logstore.c logstore.h io.c io.h coding.c coding.h hash_map.c hash_map.h siphash.c)

add_executable(bench
        bench.c
        db.c db.h logstore.c logstore.h io.c io.h coding.c coding.h hash_map.c hash_map.h siphash.c)