#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "asm.h"
#include "asm/parser.h"
#include "test.h"
#include "ut.h"

static int dp_and(void)
{
    /* Syntax error */
    ASSERT(asm_test("and r0, r1", 0) == PARSER_ERR_SYNTAX);
    ASSERT(asm_test("and r0, r1, x #0", 0) == PARSER_ERR_SYNTAX);
    ASSERT(asm_test("and r0, r1, r2, x #0", 0) == PARSER_ERR_SYNTAX);
    /* Translate to same opcode */
    ASSERT(asm_test("and r0, r1, r2", 0xe0010002) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, r2, asl #0", 0xe0010002) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, r2, lsl #0", 0xe0010002) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, r2, lsr #0", 0xe0010002) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, r2, asr #0", 0xe0010002) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, r2, ror #0", 0xe0010002) == PARSER_OK);
    /* ASL */
    ASSERT(asm_test("and r0, r1, r2, asl #1", 0xe0010082) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, r2, asl #31", 0xe0010f82) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, r2, asl #32", 0) == PARSER_ERR_SHIFT);
    ASSERT(asm_test("and r0, r1, r2, asl r1", 0xe0010112) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, r2, asl r15", 0xe0010f12) == PARSER_OK);
    /* LSL */
    ASSERT(asm_test("and r0, r1, r2, lsl #1", 0xe0010082) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, r2, lsl #31", 0xe0010f82) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, r2, lsl #32", 0) == PARSER_ERR_SHIFT);
    ASSERT(asm_test("and r0, r1, r2, lsl r1", 0xe0010112) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, r2, lsl r15", 0xe0010f12) == PARSER_OK);
    /* ASR */
    ASSERT(asm_test("and r0, r1, r2, asr #1", 0xe00100c2) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, r2, asr #31", 0xe0010fc2) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, r2, asr #32", 0) == PARSER_ERR_SHIFT);
    ASSERT(asm_test("and r0, r1, r2, asr r1", 0xe0010152) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, r2, asr r15", 0xe0010f52) == PARSER_OK);
    /* ROR */
    ASSERT(asm_test("and r0, r1, r2, ror #1", 0xe00100e2) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, r2, ror #31", 0xe0010fe2) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, r2, ror #32", 0) == PARSER_ERR_SHIFT);
    ASSERT(asm_test("and r0, r1, r2, ror r1", 0xe0010172) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, r2, ror r15", 0xe0010f72) == PARSER_OK);
    return 0;
}

void parser_test(void)
{
    ut_run(dp_and);
}
