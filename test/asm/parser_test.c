#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "asm/parser.h"
#include "test.h"
#include "ut.h"

static uint32_t asm_to_opcode(const char *src, size_t size)
{
    int ret;
    parser_t p;
    uint32_t buf;
    FILE *in = fmemopen((void *)src, size, "r");
    FILE *out = fmemopen(&buf, sizeof(buf), "w");

    if (in == NULL) {
        fprintf(stderr, "in == NULL");
        return 0;
    }
    if (out == NULL) {
        fprintf(stderr, "out == NULL");
        fclose(in);
        return 0;
    }
    parser_init(&p, in, out);
    ret = parser_exec(&p);
    parser_print_error(&p, ret);
    fclose(in);
    fclose(out);
    if (ret != PARSER_OK)
        return 0;
    return buf;
}

#define ASM_TO_OP(src) asm_to_opcode(src, sizeof(src))

static int and_lsl_imm(void)
{
    ASSERT(ASM_TO_OP("AND R0, R1, R2, LSL #0") ==  0x10002);
    return 0;
}

void parser_test(void)
{
    ut_run(and_lsl_imm);
}
