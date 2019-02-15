cmake_minimum_required(VERSION 2.8)
project(gusgba C)

SET (WARNINGS "-Wall -Wextra -pedantic -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -Wnested-externs -Winline -Wno-long-long -Wuninitialized -Wstrict-prototypes -Wconversion")

set (CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -Wall -std=gnu11 -O2 -fno-strict-aliasing ${WARNINGS}")
include_directories(${PROJECT_SOURCE_DIR}/src)

add_executable(gusgba
    src/arm7.c
    src/mmu.c
    src/main.c
)
