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
    arm_psr_t f = {.psr = (flags | default_flags)};
    mem_pos = 0;
    ASSERT(asm_to_opcode(src, memory, sizeof(memory)) == 0);
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
    ASSERT(test_arm_rd("subs r0, r1, r2", R0, 0xffffffff, N | C) == 0);
    ASSERT(test_arm_rd("subs r0, r9, r8", R0, 0x80000000, N | C | V) == 0);
    ASSERT(test_arm_rd("subs r0, r10, r1", R0, 0x7fffffff, V) == 0);
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
    ASSERT(test_arm_rd("rsbs r0, r2, r1", R0, 0xffffffff, N | C) == 0);
    ASSERT(test_arm_rd("rsbs r0, r8, r9", R0, 0x80000000, N | C | V) == 0);
    ASSERT(test_arm_rd("rsbs r0, r1, r10", R0, 0x7fffffff, V) == 0);
    return 0;
}

static int arm_add_test(void)
{
    arm_reset();
    ASSERT(test_arm_rd("add r0, r1, r8", R0, 0x00000000, 0) == 0);
    ASSERT(test_arm_rd("add r0, r8, r8", R0, 0xfffffffe, 0) == 0);
    ASSERT(test_arm_rd("adds r0, r1, r1", R0, 0x00000002, 0) == 0);
    ASSERT(test_arm_rd("adds r0, r1, r8", R0, 0x00000000, Z | C) == 0);
    ASSERT(test_arm_rd("adds r0, r8, r8", R0, 0xfffffffe, N | C) == 0);
    ASSERT(test_arm_rd("adds r0, r9, r1", R0, 0x80000000, N | V) == 0);
    ASSERT(test_arm_rd("adds r0, r10, r8", R0, 0x7fffffff, C | V) == 0);
    return 0;
}

static int arm_adc_test(void)
{
    arm_reset();
    ASSERT(test_arm_rd("adc r0, r1, r8", R0, 0x00000000, 0) == 0);
    ASSERT(test_arm_rd("adc r0, r8, r8", R0, 0xfffffffe, 0) == 0);
    ASSERT(test_arm_rd("adcs r0, r1, r1", R0, 0x00000002, 0) == 0);
    ASSERT(test_arm_rd("adcs r0, r1, r8", R0, 0x00000000, Z | C) == 0);
    ASSERT(test_arm_rd("adcs r0, r8, r8", R0, 0xffffffff, N | C) == 0);
    ASSERT(test_arm_rd("adcs r0, r9, r1", R0, 0x80000001, N | V) == 0);
    ASSERT(test_arm_rd("adcs r0, r10, r8", R0, 0x7fffffff, C | V) == 0);
    return 0;
}

static int arm_sbc_test(void)
{
    arm_reset();
    ASSERT(test_arm_rd("sbc r0, r0, r0", R0, 0xffffffff, 0) == 0);
    ASSERT(test_arm_rd("sbc r0, r1, r2", R0, 0xfffffffe, 0) == 0);
    ASSERT(test_arm_rd("sbcs r0, r0, r0", R0, 0xffffffff, N | C) == 0);
    ASSERT(test_arm_rd("sbcs r0, r1, r8", R0, 0x00000002, C) == 0);
    ASSERT(test_arm_rd("sbcs r0, r2, r1", R0, 0x00000001, 0) == 0);
    ASSERT(test_arm_rd("sbcs r0, r1, r2", R0, 0xfffffffe, N | C) == 0);
    ASSERT(test_arm_rd("sbcs r0, r9, r8", R0, 0x80000000, N | C | V) == 0);
    ASSERT(test_arm_rd("sbcs r0, r10, r1", R0, 0x7fffffff, V) == 0);
    return 0;
}

static int arm_rsc_test(void)
{
    arm_reset();
    ASSERT(test_arm_rd("rsc r0, r0, r0", R0, 0xffffffff, 0) == 0);
    ASSERT(test_arm_rd("rsc r0, r2, r1", R0, 0xfffffffe, 0) == 0);
    ASSERT(test_arm_rd("rscs r0, r0, r0", R0, 0xffffffff, N | C) == 0);
    ASSERT(test_arm_rd("rscs r0, r0, r1", R0, 0x00000002, C) == 0);
    ASSERT(test_arm_rd("rscs r0, r1, r2", R0, 0x00000001, 0) == 0);
    ASSERT(test_arm_rd("rscs r0, r2, r1", R0, 0xfffffffe, N | C) == 0);
    ASSERT(test_arm_rd("rscs r0, r8, r9", R0, 0x80000000, N | C | V) == 0);
    ASSERT(test_arm_rd("rscs r0, r1, r10", R0, 0x7fffffff, V) == 0);
    return 0;
}

static int arm_tst_test(void)
{
    arm_reset();
    arm.r[0] = 0x00;
    /* basic */
    ASSERT(test_arm_rd("tst r0, r0", R0, 0x00000000, Z) == 0);
    ASSERT(test_arm_rd("tst r1, r3", R0, 0x00000000, 0) == 0);
    ASSERT(test_arm_rd("tst r4, r5", R0, 0x00000000, 0) == 0);
    ASSERT(test_arm_rd("tst r7, r8", R0, 0x00000000, 0) == 0);
    /* LSL imm */
    ASSERT(test_arm_rd("tst r2, r1, lsl #1", R0, 0x00000000, 0) == 0);
    ASSERT(test_arm_rd("tst r8, r1, lsl #31", R0, 0x00000000, N) == 0);
    /* LSR imm */
    ASSERT(test_arm_rd("tst r1, r2, lsr #1", R0, 0x00000000, 0) == 0);
    ASSERT(test_arm_rd("tst r3, r4, lsr #1", R0, 0x00000000, 0) == 0);
    ASSERT(test_arm_rd("tst r8, r8, lsr #32", R0, 0x00000000, Z | C) == 0);
    /* ASR imm */
    ASSERT(test_arm_rd("tst r8, r8, asr #1", R0, 0x00000000, N | C) == 0);
    ASSERT(test_arm_rd("tst r7, r8, asr #31", R0, 0x00000000, C) == 0);
    ASSERT(test_arm_rd("tst r9, r9, asr #1", R0, 0x00000000, 0) == 0);
    ASSERT(test_arm_rd("tst r8, r9, asr #31", R0, 0x00000000, Z) == 0);
    /* ROR imm */
    ASSERT(test_arm_rd("tst r8, r8, ror #1", R0, 0x00000000, N | C) == 0);
    ASSERT(test_arm_rd("tst r8, r8, ror #31", R0, 0x00000000, N | C) == 0);
    ASSERT(test_arm_rd("tst r8, r7, ror #4", R0, 0x00000000, 0) == 0);
    /* LSL reg */
    ASSERT(test_arm_rd("tst r2, r1, lsl r1", R0, 0x00000000, 0) == 0);
    ASSERT(test_arm_rd("tst r8, r1, lsl r3", R0, 0x00000000, 0) == 0);
    /* LSR reg */
    ASSERT(test_arm_rd("tst r1, r2, lsr r1", R0, 0x00000000, 0) == 0);
    ASSERT(test_arm_rd("tst r3, r4, lsr r1", R0, 0x00000000, 0) == 0);
    /* ASR reg */
    ASSERT(test_arm_rd("tst r8, r8, asr r1", R0, 0x00000000, N | C) == 0);
    ASSERT(test_arm_rd("tst r7, r8, asr r8", R0, 0x00000000, C) == 0);
    ASSERT(test_arm_rd("tst r9, r9, asr r1", R0, 0x00000000, 0) == 0);
    ASSERT(test_arm_rd("tst r8, r9, asr r8", R0, 0x00000000, Z) == 0);
    /* ROR reg */
    ASSERT(test_arm_rd("tst r8, r8, ror r1", R0, 0x00000000, N | C) == 0);
    ASSERT(test_arm_rd("tst r8, r8, ror r8", R0, 0x00000000, N | C) == 0);
    ASSERT(test_arm_rd("tst r8, r7, ror r4", R0, 0x00000000, 0) == 0);

    return 0;
}

static int arm_teq_test(void)
{
    arm_reset();
    ASSERT(test_arm_rd("teq r0, r0", R0, 0x00000000, Z) == 0);
    ASSERT(test_arm_rd("teq r0, r1", R0, 0x00000000, 0) == 0);
    ASSERT(test_arm_rd("teq r1, r2", R0, 0x00000000, 0) == 0);
    ASSERT(test_arm_rd("teq r1, r8", R0, 0x00000000, N) == 0);
    return 0;
}

static int arm_cmp_test(void)
{
    arm_reset();
    ASSERT(test_arm_rd("cmp r0, r0", R0, 0x00000000, Z) == 0);
    ASSERT(test_arm_rd("cmp r1, r0", R0, 0x00000000, 0) == 0);
    ASSERT(test_arm_rd("cmp r2, r1", R0, 0x00000000, 0) == 0);
    ASSERT(test_arm_rd("cmp r1, r2", R0, 0x00000000, N | C) == 0);
    ASSERT(test_arm_rd("cmp r9, r8", R0, 0x00000000, N | C | V) == 0);
    ASSERT(test_arm_rd("cmp r10, r1", R0, 0x00000000, V) == 0);
    return 0;
}

static int arm_cmn_test(void)
{
    arm_reset();
    ASSERT(test_arm_rd("cmn r1, r1", R0, 0x00000000, 0) == 0);
    ASSERT(test_arm_rd("cmn r1, r8", R0, 0x00000000, Z | C) == 0);
    ASSERT(test_arm_rd("cmn r8, r8", R0, 0x00000000, N | C) == 0);
    ASSERT(test_arm_rd("cmn r9, r1", R0, 0x00000000, N | V) == 0);
    ASSERT(test_arm_rd("cmn r10, r8", R0, 0x00000000, C | V) == 0);
    return 0;
}

static int arm_orr_test(void)
{
    arm_reset();
    ASSERT(test_arm_rd("orr r0, r0, r0", R0, 0x00000000, 0) == 0);
    ASSERT(test_arm_rd("orr r0, r1, r8", R0, 0xffffffff, 0) == 0);
    ASSERT(test_arm_rd("orrs r0, r1, r1", R0, 0x00000001, 0) == 0);
    ASSERT(test_arm_rd("orrs r0, r0, r1", R0, 0x00000001, 0) == 0);
    ASSERT(test_arm_rd("orrs r0, r1, r2", R0, 0x00000003, 0) == 0);
    ASSERT(test_arm_rd("orrs r0, r1, r8", R0, 0xffffffff, N) == 0);
    return 0;
}

static int arm_mov_test(void)
{
    arm_reset();
    ASSERT(test_arm_rd("mov r0, r8", R0, 0xffffffff, 0) == 0);
    ASSERT(test_arm_rd("movs r0, r1", R0, 0x00000001, 0) == 0);
    ASSERT(test_arm_rd("movs r0, r8", R0, 0xffffffff, N) == 0);
    ASSERT(test_arm_rd("movs r0, r8, lsl #1", R0, 0xfffffffe, N | C) == 0);
    ASSERT(test_arm_rd("movs r0, r1, lsr #1", R0, 0x00000000, Z | C) == 0);
    return 0;
}

static int arm_bic_test(void)
{
    arm_reset();
    ASSERT(test_arm_rd("bic r0, r8, r1", R0, 0xfffffffe, 0) == 0);
    ASSERT(test_arm_rd("bics r0, r8, r1", R0, 0xfffffffe, N) == 0);
    ASSERT(test_arm_rd("bics r0, r1, r8, lsl #1", R0, 0x00000001, C) == 0);
    return 0;
}

static int arm_mvn_test(void)
{
    arm_reset();
    ASSERT(test_arm_rd("mvn r0, r1", R0, 0xfffffffe, 0) == 0);
    ASSERT(test_arm_rd("mvns r0, r1", R0, 0xfffffffe, N) == 0);
    ASSERT(test_arm_rd("mvns r0, r8, lsl #1", R0, 0x00000001, C) == 0);
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
    arm.r[10] = 0x80000000;
    ut_run(arm_and_test);
    ut_run(arm_eor_test);
    ut_run(arm_sub_test);
    ut_run(arm_rsb_test);
    ut_run(arm_add_test);
    ut_run(arm_adc_test);
    ut_run(arm_sbc_test);
    ut_run(arm_rsc_test);
    ut_run(arm_tst_test);
    ut_run(arm_teq_test);
    ut_run(arm_cmp_test);
    ut_run(arm_cmn_test);
    ut_run(arm_orr_test);
    ut_run(arm_mov_test);
    ut_run(arm_bic_test);
    ut_run(arm_mvn_test);
}
