#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "asm/parser.h"
#include "asm.h"
#include "test.h"
#include "ut.h"

static int dp_and(void)
{
    ASSERT(asm_test("AND R0, R1", 0) == PARSER_ERR_SYNTAX);
    ASSERT(asm_test("AND R0, R1, R2", 0) == PARSER_ERR_SYNTAX);
    ASSERT(asm_test("AND R0, R1, X #0", 0) == PARSER_ERR_SYNTAX);
    ASSERT(asm_test("AND R0, R1, R2, X #0", 0) == PARSER_ERR_SYNTAX);
    ASSERT(asm_test("AND R0, R1, R2, ASL #0", 0xe0010002) == PARSER_OK);
    ASSERT(asm_test("AND R0, R1, R2, LSL #0", 0xe0010002) == PARSER_OK);
    ASSERT(asm_test("AND R0, R1, R2, LSR #0", 0xe0010022) == PARSER_OK);
    ASSERT(asm_test("AND R0, R1, R2, ASR #0", 0xe0010042) == PARSER_OK);
    ASSERT(asm_test("AND R0, R1, R2, ROR #0", 0xe0010062) == PARSER_OK);
    return 0;
}

void parser_test(void)
{
    ut_run(dp_and);
}
