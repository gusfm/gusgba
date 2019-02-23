#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "asm/parser.h"
#include "test.h"
#include "ut.h"

static int and_lsl_imm(void)
{
    parser_t p;
    char src[] = "AND R0, R1, R2, LSL #0";
    uint8_t buf[100];
    FILE *in = fmemopen(src, sizeof(src) - 1, "r");
    FILE *out = fmemopen(buf, sizeof(buf) - 1, "w");
    ASSERT(in != NULL);
    ASSERT(parser_init(&p, in, out) == 0);
    ASSERT(parser_exec(&p) == PARSER_OK);
    fclose(in);
    return 0;
}

void parser_test(void)
{
    ut_run(and_lsl_imm);
}
