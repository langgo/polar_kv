cmake_minimum_required(VERSION 3.9)
project(polar_kv)

set(CMAKE_CXX_STANDARD 11)

add_executable(polar_kv
        main.c db.c db.h logstore.c logstore.h io.c io.h coding.c coding.h)
