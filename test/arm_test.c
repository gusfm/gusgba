#include <string.h>

#include "arm.h"
#include "asm/asm.h"
#include "mmu.h"
#include "test.h"
#include "ut.h"

#define V ARM_PSR_OVERFLOW
#define C ARM_PSR_CARRY
#define Z ARM_PSR_ZERO
#define N ARM_PSR_NEGATIVE

static uint8_t memory[0x10];
static int mem_pos;
static uint32_t default_flags =
    ARM_PSR_IRQ_DISABLE | ARM_PSR_FIQ_DISABLE | ARM_PSR_SVC_MODE;

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

static int test_arm_rd(const char *src, int rd, uint32_t rd_val, uint32_t flags)
{
    arm_psr_t f = { .psr = (flags | default_flags) };
    mem_pos = 0;
    asm_to_opcode(src, memory, sizeof(memory));
    arm_step();
    ASSERT_EQ(rd_val, arm.r[rd]);
    ASSERT_EQ(f.mode, arm.cpsr.mode);
    ASSERT_EQ(f.t, arm.cpsr.t);
    ASSERT_EQ(f.f, arm.cpsr.f);
    ASSERT_EQ(f.i, arm.cpsr.i);
    ASSERT_EQ(f.v, arm.cpsr.v);
    ASSERT_EQ(f.c, arm.cpsr.c);
    ASSERT_EQ(f.z, arm.cpsr.z);
    ASSERT_EQ(f.n, arm.cpsr.n);
    return 0;
}

static int arm_and_test(void)
{
    /* basic */
    ASSERT(test_arm_rd("ands r0, r0, r0", R0, 0x00000000, Z) == 0);
    ASSERT(test_arm_rd("ands r0, r1, r3", R0, 0x00000001, 0) == 0);
    ASSERT(test_arm_rd("ands r0, r4, r5", R0, 0x00000004, 0) == 0);
    ASSERT(test_arm_rd("ands r0, r7, r8", R0, 0x00000007, 0) == 0);
    /* LSL imm */
    ASSERT(test_arm_rd("ands r0, r2, r1, lsl #1", R0, 0x00000002, 0) == 0);
    ASSERT(test_arm_rd("ands r0, r8, r1, lsl #31", R0, 0x80000000, N) == 0);
    /* LSR imm */
    ASSERT(test_arm_rd("ands r0, r1, r2, lsr #1", R0, 0x00000001, 0) == 0);
    ASSERT(test_arm_rd("ands r0, r3, r4, lsr #1", R0, 0x00000002, 0) == 0);
    ASSERT(test_arm_rd("ands r0, r8, r8, lsr #32", R0, 0x00000000, Z | C) == 0);
    /* ASR imm */
    ASSERT(test_arm_rd("ands r0, r8, r8, asr #1", R0, 0xffffffff, N | C) == 0);
    ASSERT(test_arm_rd("ands r0, r7, r8, asr #31", R0, 0x00000007, C) == 0);
    ASSERT(test_arm_rd("ands r0, r9, r9, asr #1", R0, 0x3fffffff, 0) == 0);
    ASSERT(test_arm_rd("ands r0, r8, r9, asr #31", R0, 0x00000000, Z) == 0);
    /* ROR imm */
    ASSERT(test_arm_rd("ands r0, r8, r8, ror #1", R0, 0xffffffff, N | C) == 0);
    ASSERT(test_arm_rd("ands r0, r8, r8, ror #31", R0, 0xffffffff, N | C) == 0);
    ASSERT(test_arm_rd("ands r0, r8, r7, ror #4", R0, 0x70000000, 0) == 0);
    /* LSL reg */
    ASSERT(test_arm_rd("ands r0, r2, r1, lsl r1", R0, 0x00000002, 0) == 0);
    ASSERT(test_arm_rd("ands r0, r8, r1, lsl r3", R0, 0x00000008, 0) == 0);
    /* LSR reg */
    ASSERT(test_arm_rd("ands r0, r1, r2, lsr r1", R0, 0x00000001, 0) == 0);
    ASSERT(test_arm_rd("ands r0, r3, r4, lsr r1", R0, 0x00000002, 0) == 0);
    /* ASR reg */
    ASSERT(test_arm_rd("ands r0, r8, r8, asr r1", R0, 0xffffffff, N | C) == 0);
    ASSERT(test_arm_rd("ands r0, r7, r8, asr r8", R0, 0x00000007, C) == 0);
    ASSERT(test_arm_rd("ands r0, r9, r9, asr r1", R0, 0x3fffffff, 0) == 0);
    ASSERT(test_arm_rd("ands r0, r8, r9, asr r8", R0, 0x00000000, Z) == 0);
    /* ROR reg */
    ASSERT(test_arm_rd("ands r0, r8, r8, ror r1", R0, 0xffffffff, N | C) == 0);
    ASSERT(test_arm_rd("ands r0, r8, r8, ror r8", R0, 0xffffffff, N | C) == 0);
    ASSERT(test_arm_rd("ands r0, r8, r7, ror r4", R0, 0x70000000, 0) == 0);

    return 0;
}

static int arm_eor_test(void)
{
    arm_reset();
    ASSERT(test_arm_rd("eor r0, r0, r0", R0, 0x00000000, 0) == 0);
    ASSERT(test_arm_rd("eor r0, r1, r8", R0, 0xfffffffe, 0) == 0);
    ASSERT(test_arm_rd("eors r0, r0, r0", R0, 0x00000000, Z) == 0);
    ASSERT(test_arm_rd("eors r0, r0, r1", R0, 0x00000001, 0) == 0);
    ASSERT(test_arm_rd("eors r0, r1, r2", R0, 0x00000003, 0) == 0);
    ASSERT(test_arm_rd("eors r0, r1, r8", R0, 0xfffffffe, N) == 0);
    return 0;
}

static int arm_sub_test(void)
{
    arm_reset();
    ASSERT(test_arm_rd("sub r0, r0, r0", R0, 0x00000000, 0) == 0);
    ASSERT(test_arm_rd("sub r0, r1, r2", R0, 0xffffffff, 0) == 0);
    ASSERT(test_arm_rd("subs r0, r0, r0", R0, 0x00000000, Z) == 0);
    ASSERT(test_arm_rd("subs r0, r1, r0", R0, 0x00000001, 0) == 0);
    ASSERT(test_arm_rd("subs r0, r2, r1", R0, 0x00000001, 0) == 0);
    ASSERT(test_arm_rd("subs r0, r1, r2", R0, 0xffffffff, N) == 0);
    return 0;
}

static int arm_rsb_test(void)
{
    arm_reset();
    ASSERT(test_arm_rd("rsb r0, r0, r0", R0, 0x00000000, 0) == 0);
    ASSERT(test_arm_rd("rsb r0, r2, r1", R0, 0xffffffff, 0) == 0);
    ASSERT(test_arm_rd("rsbs r0, r0, r0", R0, 0x00000000, Z) == 0);
    ASSERT(test_arm_rd("rsbs r0, r0, r1", R0, 0x00000001, 0) == 0);
    ASSERT(test_arm_rd("rsbs r0, r1, r2", R0, 0x00000001, 0) == 0);
    ASSERT(test_arm_rd("rsbs r0, r2, r1", R0, 0xffffffff, N) == 0);
    return 0;
}

static int arm_add_test(void)
{
    arm_reset();
    ASSERT(test_arm_rd("add r0, r1, r8", R0, 0x00000000, 0) == 0);
    ASSERT(test_arm_rd("add r0, r8, r8", R0, 0xfffffffe, 0) == 0);
    ASSERT(test_arm_rd("adds r0, r1, r1", R0, 0x00000002, 0) == 0);
    ASSERT(test_arm_rd("adds r0, r1, r8", R0, 0x00000000, Z) == 0);
    ASSERT(test_arm_rd("adds r0, r8, r8", R0, 0xfffffffe, N) == 0);
    return 0;
}

void arm_test(void)
{
    arm_init();
    arm.r[1] = 0x01;
    arm.r[2] = 0x02;
    arm.r[3] = 0x03;
    arm.r[4] = 0x04;
    arm.r[5] = 0x05;
    arm.r[6] = 0x06;
    arm.r[7] = 0x07;
    arm.r[8] = 0xffffffff;
    arm.r[9] = 0x7fffffff;
    ut_run(arm_and_test);
    ut_run(arm_eor_test);
    ut_run(arm_sub_test);
    ut_run(arm_rsb_test);
    ut_run(arm_add_test);
}
