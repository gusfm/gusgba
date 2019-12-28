cmake_minimum_required(VERSION 2.8)
project(gusgba C)

SET (WARNINGS "-Wall -Wextra -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -Wnested-externs -Winline -Wno-long-long -Wuninitialized -Wstrict-prototypes -Wconversion")

set (CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -Wall -std=gnu11 -O2 -fno-strict-aliasing ${WARNINGS}")
include_directories(${PROJECT_SOURCE_DIR}/src)

add_executable(gusgba
    src/arm_isa.c
    src/arm.c
    src/arm_debug.c
    src/mmu.c
    src/main.c
)

add_library(asbase SHARED
    src/asm/str.c
    src/asm/token.c
    src/asm/lex.c
    src/asm/arm_enc.c
    src/asm/parser.c
    )

enable_testing()

include_directories(${PROJECT_SOURCE_DIR}/test)

add_executable(gusgbatest
    src/arm_isa.c
    src/arm.c
    src/arm_debug.c
    test/arm_test.c
    test/asm/asm.c
    test/asm/lex_test.c
    test/asm/parser_test.c
    test/main.c
    )
target_link_libraries(gusgbatest
    asbase
    )
add_test(test gusgbatest)
