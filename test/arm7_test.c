#include <string.h>

#include "arm7.h"
#include "asm/asm.h"
#include "mmu.h"
#include "test.h"
#include "ut.h"

static uint8_t memory[0x10];
static int mem_pos;

uint32_t mmu_read_word(uint32_t addr)
{
    (void)addr;
    uint32_t val = *(uint32_t *)&memory[mem_pos];
    mem_pos += 4;
    return val;
}

uint16_t mmu_read_half_word(uint32_t addr)
{
    (void)addr;
    uint16_t val = *(uint16_t *)&memory[mem_pos];
    mem_pos += 2;
    return val;
}

static uint32_t test_arm_rd(const char *src, int rd)
{
    mem_pos = 0;
    asm_to_opcode(src, memory, sizeof(memory));
    arm7_step();
    return arm.r[rd];
}

static int arm7_and_test(void)
{
    /* basic */
    ASSERT_EQ(0x00000000, test_arm_rd("and r0, r0, r0", R0));
    ASSERT_EQ(0x00000001, test_arm_rd("and r0, r1, r3", R0));
    ASSERT_EQ(0x00000004, test_arm_rd("and r0, r4, r5", R0));
    ASSERT_EQ(0x00000007, test_arm_rd("and r0, r7, r8", R0));
    /* LSL imm */
    ASSERT_EQ(0x00000002, test_arm_rd("and r0, r2, r1, lsl #1", R0));
    ASSERT_EQ(0x80000000, test_arm_rd("and r0, r8, r1, lsl #31", R0));
    /* LSR imm */
    ASSERT_EQ(0x00000001, test_arm_rd("and r0, r1, r2, lsr #1", R0));
    ASSERT_EQ(0x00000002, test_arm_rd("and r0, r3, r4, lsr #1", R0));
    ASSERT_EQ(0x00000000, test_arm_rd("and r0, r8, r8, lsr #32", R0));
    /* ASR imm */
    ASSERT_EQ(0xffffffff, test_arm_rd("and r0, r8, r8, asr #1", R0));
    ASSERT_EQ(0x00000007, test_arm_rd("and r0, r7, r8, asr #31", R0));
    ASSERT_EQ(0x3fffffff, test_arm_rd("and r0, r9, r9, asr #1", R0));
    ASSERT_EQ(0x00000000, test_arm_rd("and r0, r8, r9, asr #31", R0));
    /* ROR imm */
    ASSERT_EQ(0xffffffff, test_arm_rd("and r0, r8, r8, ror #1", R0));
    ASSERT_EQ(0xffffffff, test_arm_rd("and r0, r8, r8, ror #31", R0));
    ASSERT_EQ(0x70000000, test_arm_rd("and r0, r8, r7, ror #4", R0));
    /* LSL reg */
    ASSERT_EQ(0x00000002, test_arm_rd("and r0, r2, r1, lsl r1", R0));
    ASSERT_EQ(0x00000008, test_arm_rd("and r0, r8, r1, lsl r3", R0));
    /* LSR reg */
    ASSERT_EQ(0x00000001, test_arm_rd("and r0, r1, r2, lsr r1", R0));
    ASSERT_EQ(0x00000002, test_arm_rd("and r0, r3, r4, lsr r1", R0));
    /* ASR reg */
    ASSERT_EQ(0xffffffff, test_arm_rd("and r0, r8, r8, asr r1", R0));
    ASSERT_EQ(0x00000007, test_arm_rd("and r0, r7, r8, asr r8", R0));
    ASSERT_EQ(0x3fffffff, test_arm_rd("and r0, r9, r9, asr r1", R0));
    ASSERT_EQ(0x00000000, test_arm_rd("and r0, r8, r9, asr r8", R0));
    /* ROR reg */
    ASSERT_EQ(0xffffffff, test_arm_rd("and r0, r8, r8, ror r1", R0));
    ASSERT_EQ(0xffffffff, test_arm_rd("and r0, r8, r8, ror r8", R0));
    ASSERT_EQ(0x70000000, test_arm_rd("and r0, r8, r7, ror r4", R0));

    return 0;
}

void arm7_test(void)
{
    arm.r[1] = 0x01;
    arm.r[2] = 0x02;
    arm.r[3] = 0x03;
    arm.r[4] = 0x04;
    arm.r[5] = 0x05;
    arm.r[6] = 0x06;
    arm.r[7] = 0x07;
    arm.r[8] = 0xffffffff;
    arm.r[9] = 0x7fffffff;
    ut_run(arm7_and_test);
}
