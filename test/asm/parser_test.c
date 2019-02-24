#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "asm/parser.h"
#include "test.h"
#include "ut.h"

#define TEST_ASM(src, expected_op) test_asm(src, sizeof(src), expected_op)

static int test_asm(const char *src, size_t size, uint32_t opcode)
{
    int ret;
    parser_t p;
    uint32_t buf = 0;
    FILE *in = fmemopen((void *)src, size, "r");
    FILE *out = fmemopen(&buf, sizeof(buf), "w");
    ASSERT(in != NULL);
    ASSERT(out != NULL);
    ASSERT(parser_init(&p, in, out) == 0);
    ret = parser_exec(&p);
    parser_print_error(&p, ret);
    fclose(in);
    fclose(out);
    ASSERT_EQ(opcode, buf);
    return ret;
}

static int dp_and(void)
{
    ASSERT(TEST_ASM("AND R0, R1", 0x00000) == PARSER_ERR_SYNTAX);
    ASSERT(TEST_ASM("AND R0, R1, R2", 0x00000) == PARSER_ERR_SYNTAX);
    ASSERT(TEST_ASM("AND R0, R1, X #0", 0x00000) == PARSER_ERR_SYNTAX);
    ASSERT(TEST_ASM("AND R0, R1, R2, X #0", 0x00000) == PARSER_ERR_SYNTAX);
    ASSERT(TEST_ASM("AND R0, R1, R2, ASL #0", 0x10002) == PARSER_OK);
    ASSERT(TEST_ASM("AND R0, R1, R2, LSL #0", 0x10002) == PARSER_OK);
    ASSERT(TEST_ASM("AND R0, R1, R2, LSR #0", 0x10022) == PARSER_OK);
    ASSERT(TEST_ASM("AND R0, R1, R2, ASR #0", 0x10042) == PARSER_OK);
    ASSERT(TEST_ASM("AND R0, R1, R2, ROR #0", 0x10062) == PARSER_OK);
    return 0;
}

void parser_test(void)
{
    ut_run(dp_and);
}
