#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "asm.h"
#include "asm/parser.h"
#include "test.h"
#include "ut.h"

static int registers(void)
{
    ASSERT(asm_test("and r0, r1, r2, lsl r3", 0xe0010312) == PARSER_OK);
    ASSERT(asm_test("and r4, r5, r6, lsl r7", 0xe0054716) == PARSER_OK);
    ASSERT(asm_test("and r8, r9, r10, lsl r11", 0xe0098b1a) == PARSER_OK);
    ASSERT(asm_test("and r8, r9, sl, lsl fp", 0xe0098b1a) == PARSER_OK);
    ASSERT(asm_test("and r12, r13, r14, lsl r15", 0xe00dcf1e) == PARSER_OK);
    ASSERT(asm_test("and ip, sp, lr, lsl pc", 0xe00dcf1e) == PARSER_OK);
    return 0;
}

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
    /* LSR */
    ASSERT(asm_test("and r0, r1, r2, lsr #1", 0xe00100a2) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, r2, lsr #31", 0xe0010fa2) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, r2, lsr #32", 0xe0010022) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, r2, lsr #33", 0) == PARSER_ERR_SHIFT);
    ASSERT(asm_test("and r0, r1, r2, lsr r1", 0xe0010132) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, r2, lsr r15", 0xe0010f32) == PARSER_OK);
    /* ASR */
    ASSERT(asm_test("and r0, r1, r2, asr #1", 0xe00100c2) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, r2, asr #31", 0xe0010fc2) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, r2, asr #32", 0xe0010042) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, r2, asr #33", 0) == PARSER_ERR_SHIFT);
    ASSERT(asm_test("and r0, r1, r2, asr r1", 0xe0010152) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, r2, asr r15", 0xe0010f52) == PARSER_OK);
    /* ROR */
    ASSERT(asm_test("and r0, r1, r2, ror #1", 0xe00100e2) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, r2, ror #31", 0xe0010fe2) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, r2, ror #32", 0) == PARSER_ERR_SHIFT);
    ASSERT(asm_test("and r0, r1, r2, ror r1", 0xe0010172) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, r2, ror r15", 0xe0010f72) == PARSER_OK);
    /* Immediate value */
    ASSERT(asm_test("and r0, r1, #0", 0xe2010000) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, #1", 0xe2010001) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, #101", 0xe2010065) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, #0xff", 0xe20100ff) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, #0xfff", 0) == PARSER_ERR_INVALID_CONSTANT);
    ASSERT(asm_test("and r0, r1, #0xff00", 0xe2010cff) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, #0xff0000", 0xe20108ff) == PARSER_OK);
    ASSERT(asm_test("and r0, r1, #0xff000000", 0xe20104ff) == PARSER_OK);
    return 0;
}

void parser_test(void)
{
    ut_run(registers);
    ut_run(dp_and);
}
