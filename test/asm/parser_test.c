#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "asm/parser.h"
#include "asm.h"
#include "test.h"
#include "ut.h"

static int dp_and(void)
{
    /* Syntax error */
    ASSERT(asm_test("AND R0, R1", 0) == PARSER_ERR_SYNTAX);
    ASSERT(asm_test("AND R0, R1, X #0", 0) == PARSER_ERR_SYNTAX);
    ASSERT(asm_test("AND R0, R1, R2, X #0", 0) == PARSER_ERR_SYNTAX);
    /* Translate to same opcode */
    ASSERT(asm_test("AND R0, R1, R2", 0xe0010002) == PARSER_OK);
    ASSERT(asm_test("AND R0, R1, R2, ASL #0", 0xe0010002) == PARSER_OK);
    ASSERT(asm_test("AND R0, R1, R2, LSL #0", 0xe0010002) == PARSER_OK);
    ASSERT(asm_test("AND R0, R1, R2, LSR #0", 0xe0010002) == PARSER_OK);
    ASSERT(asm_test("AND R0, R1, R2, ASR #0", 0xe0010002) == PARSER_OK);
    ASSERT(asm_test("AND R0, R1, R2, ROR #0", 0xe0010002) == PARSER_OK);
    /* ASL */
    ASSERT(asm_test("AND R0, R1, R2, ASL #1", 0xe0010082) == PARSER_OK);
    ASSERT(asm_test("AND R0, R1, R2, ASL #31", 0xe0010f82) == PARSER_OK);
    ASSERT(asm_test("AND R0, R1, R2, ASL #32", 0) == PARSER_ERR_SHIFT);
    /* LSL */
    ASSERT(asm_test("AND R0, R1, R2, LSL #1", 0xe0010082) == PARSER_OK);
    ASSERT(asm_test("AND R0, R1, R2, LSL #31", 0xe0010f82) == PARSER_OK);
    ASSERT(asm_test("AND R0, R1, R2, LSL #32", 0) == PARSER_ERR_SHIFT);
    /* ASR */
    ASSERT(asm_test("AND R0, R1, R2, ASR #1", 0xe00100c2) == PARSER_OK);
    ASSERT(asm_test("AND R0, R1, R2, ASR #31", 0xe0010fc2) == PARSER_OK);
    ASSERT(asm_test("AND R0, R1, R2, ASR #32", 0) == PARSER_ERR_SHIFT);
    /* ROR */
    ASSERT(asm_test("AND R0, R1, R2, ROR #1", 0xe00100e2) == PARSER_OK);
    ASSERT(asm_test("AND R0, R1, R2, ROR #31", 0xe0010fe2) == PARSER_OK);
    ASSERT(asm_test("AND R0, R1, R2, ROR #32", 0) == PARSER_ERR_SHIFT);
    return 0;
}

void parser_test(void)
{
    ut_run(dp_and);
}
