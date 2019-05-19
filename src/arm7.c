#include "arm7.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mmu.h"

#define CPU_DEBUG /* debugs are enabled for now */
#ifdef CPU_DEBUG
#define printd(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define printd(fmt, ...) ((void)0)
#endif

arm7_t arm;

typedef enum {
    ARM7_COND_EQ, /* Z set */
    ARM7_COND_NE, /* Z clear */
    ARM7_COND_CS, /* C set */
    ARM7_COND_CC, /* C clear */
    ARM7_COND_MI, /* N set */
    ARM7_COND_PL, /* N clear */
    ARM7_COND_VS, /* V set */
    ARM7_COND_VC, /* V clear */
    ARM7_COND_HI, /* C set and Z clear */
    ARM7_COND_LS, /* C clear or Z set */
    ARM7_COND_GE, /* N equals V */
    ARM7_COND_LT, /* N not equal to V */
    ARM7_COND_GT, /* Z clear AND (N equals V) */
    ARM7_COND_LE, /* Z set OR (N not equal to V) */
    ARM7_COND_AL, /* (ignored) */
    ARM7_COND_RS, /* reserved */
} arm7_condition_t;

#define ARM7_PSR_EQ(psr) (psr & ARM7_PSR_ZERO)
#define ARM7_PSR_NE(psr) !(psr & ARM7_PSR_ZERO)
#define ARM7_PSR_CS(psr) (psr & ARM7_PSR_CARRY)
#define ARM7_PSR_CC(psr) !(psr & ARM7_PSR_CARRY)
#define ARM7_PSR_MI(psr) (psr & ARM7_PSR_NEGATIVE)
#define ARM7_PSR_PL(psr) !(psr & ARM7_PSR_NEGATIVE)
#define ARM7_PSR_VS(psr) (psr & ARM7_PSR_OVERFLOW)
#define ARM7_PSR_VC(psr) !(psr & ARM7_PSR_OVERFLOW)
#define ARM7_PSR_GE(psr) (ARM7_PSR_MI(psr) == ARM7_PSR_VS(psr))
#define ARM7_PSR_LT(psr) (ARM7_PSR_MI(psr) != ARM7_PSR_VS(psr))

#define ARM7_PSR_IS_SET(psr, flag) (psr & flag)
#define ARM7_PSR_SET(psr, flag) (psr |= (flag))
#define ARM7_PSR_CLEAR(psr, flag) (psr &= ~(flag))

static uint32_t asr_mask[32]
    = { 0xffffffff, 0x80000000, 0xc0000000, 0xe0000000, 0xf0000000, 0xf8000000,
        0xfc000000, 0xfe000000, 0xff000000, 0xff800000, 0xffc00000, 0xffe00000,
        0xfff00000, 0xfff80000, 0xfffc0000, 0xfffe0000, 0xffff0000, 0xffff8000,
        0xffffc000, 0xffffe000, 0xfffff000, 0xfffff800, 0xfffffc00, 0xfffffe00,
        0xffffff00, 0xffffff80, 0xffffffc0, 0xffffffe0, 0xfffffff0, 0xfffffff8,
        0xfffffffc, 0xfffffffe };

static uint32_t ror_mask[32]
    = { 0x00000000, 0x00000001, 0x00000003, 0x00000007, 0x0000000f, 0x0000001f,
        0x0000003f, 0x0000007f, 0x000000ff, 0x000001ff, 0x000003ff, 0x000007ff,
        0x00000fff, 0x00001fff, 0x00003fff, 0x00007fff, 0x0000ffff, 0x0001ffff,
        0x0003ffff, 0x0007ffff, 0x000fffff, 0x001fffff, 0x003fffff, 0x007fffff,
        0x00ffffff, 0x01ffffff, 0x03ffffff, 0x07ffffff, 0x0fffffff, 0x1fffffff,
        0x3fffffff, 0x7fffffff };

static bool evaluate_cond(arm7_condition_t cond, uint32_t cpsr)
{
    switch (cond) {
        case ARM7_COND_EQ:
            return ARM7_PSR_EQ(cpsr);
        case ARM7_COND_NE:
            return ARM7_PSR_NE(cpsr);
        case ARM7_COND_CS:
            return ARM7_PSR_CS(cpsr);
        case ARM7_COND_CC:
            return ARM7_PSR_CC(cpsr);
        case ARM7_COND_MI:
            return ARM7_PSR_MI(cpsr);
        case ARM7_COND_PL:
            return ARM7_PSR_PL(cpsr);
        case ARM7_COND_VS:
            return ARM7_PSR_VS(cpsr);
        case ARM7_COND_VC:
            return ARM7_PSR_VC(cpsr);
        case ARM7_COND_HI:
            return ARM7_PSR_CS(cpsr) && ARM7_PSR_NE(cpsr);
        case ARM7_COND_LS:
            return ARM7_PSR_CC(cpsr) || ARM7_PSR_EQ(cpsr);
        case ARM7_COND_GE:
            return ARM7_PSR_GE(cpsr);
        case ARM7_COND_LT:
            return ARM7_PSR_LT(cpsr);
        case ARM7_COND_GT:
            return ARM7_PSR_NE(cpsr) && ARM7_PSR_GE(cpsr);
        case ARM7_COND_LE:
            return ARM7_PSR_EQ(cpsr) || ARM7_PSR_LT(cpsr);
        case ARM7_COND_AL:
            return true;
        case ARM7_COND_RS:
        default:
            abort();
    }
}

/* Data processing logical shift left immediate */
static inline uint32_t dp_lsl_imm(uint32_t opcode)
{
    uint32_t rm = opcode & 0xf;
    uint32_t shift = (opcode >> 7) & 0x1f;
    if (shift)
        printd("r%u, lsl #%u\n", rm, shift);
    else
        printd("r%u\n", rm);
    return arm.r[rm] << shift;
}

/* Data processing logical shift right immediate */
static inline uint32_t dp_lsr_imm(uint32_t opcode)
{
    uint32_t rm = opcode & 0xf;
    uint32_t shift = (opcode >> 7) & 0x1f;
    if (!shift) {
        printd("r%u, lsr #32\n", rm);
        return 0;
    }
    printd("r%u, lsr #%u\n", rm, shift);
    return arm.r[rm] >> shift;
}

/* Data processing arithmetic shift right immediate */
static inline uint32_t dp_asr_imm(uint32_t opcode)
{
    uint32_t rm = opcode & 0xf;
    uint32_t shift = (opcode >> 7) & 0x1f;
    uint32_t val = arm.r[rm];
    uint32_t ret = val >> shift;
    if (val & 0x80000000)
        ret |= asr_mask[shift];
    printd("r%u, lsr #%u\n", rm, shift);
    return ret;
}

/* Data processing rotate right immediate */
static inline uint32_t dp_ror_imm(uint32_t opcode)
{
    uint32_t rm = opcode & 0xf;
    uint32_t shift = (opcode >> 7) & 0x1f;
    uint32_t val = arm.r[rm];
    uint32_t carry = val & ror_mask[shift];
    printd("r%u, ror #%u\n", rm, shift);
    return (carry << (32 - shift)) | (val >> shift);
}

/* Data processing logical shift left reg */
static inline uint32_t dp_lsl_reg(uint32_t opcode)
{
    uint32_t rm = opcode & 0xf;
    uint32_t rs = (opcode >> 8) & 0xf;
    uint32_t shift = (arm.r[rs] & 0x1f);
    printd("r%u, lsl r%u\n", rm, rs);
    return arm.r[rm] << shift;
}

/* Data processing logical shift right reg */
static inline uint32_t dp_lsr_reg(uint32_t opcode)
{
    uint32_t rm = opcode & 0xf;
    uint32_t rs = (opcode >> 8) & 0xf;
    uint32_t shift = (arm.r[rs] & 0x1f);
    printd("r%u, lsr r%u\n", rm, rs);
    return arm.r[rm] >> shift;
}

/* Data processing arithmetic shift right reg */
static inline uint32_t dp_asr_reg(uint32_t opcode)
{
    uint32_t rm = opcode & 0xf;
    uint32_t rs = (opcode >> 8) & 0xf;
    uint32_t shift = (arm.r[rs] & 0x1f);
    uint32_t val = arm.r[rm];
    uint32_t ret = val >> shift;
    if (val & 0x80000000)
        ret |= asr_mask[shift];
    printd("r%u, asr r%u\n", rm, rs);
    return ret;
}

/* Data processing rotate right reg */
static inline uint32_t dp_ror_reg(uint32_t opcode)
{
    uint32_t rm = opcode & 0xf;
    uint32_t rs = (opcode >> 8) & 0xf;
    uint32_t shift = (arm.r[rs] & 0x1f);
    uint32_t val = arm.r[rm];
    uint32_t carry = val & ror_mask[shift];
    printd("r%u, ror r%u\n", rm, rs);
    return (carry << (32 - shift)) | (val >> shift);
}

/* AND Rd, Rn, Rm, LSL # */
static void and_lsl_imm(uint32_t opcode)
{
    uint32_t rd = (opcode >> 12) & 0xf;
    uint32_t rn = (opcode >> 16) & 0xf;
    printd("and r%u, r%u, ", rd, rn);
    arm.r[rd] = arm.r[rn] & dp_lsl_imm(opcode);
}

/* AND Rd, Rn, Rm, LSR # */
static void and_lsr_imm(uint32_t opcode)
{
    uint32_t rd = (opcode >> 12) & 0xf;
    uint32_t rn = (opcode >> 16) & 0xf;
    printd("and r%u, r%u, ", rd, rn);
    arm.r[rd] = arm.r[rn] & dp_lsr_imm(opcode);
}

/* AND Rd, Rn, Rm, ASR # */
static void and_asr_imm(uint32_t opcode)
{
    uint32_t rd = (opcode >> 12) & 0xf;
    uint32_t rn = (opcode >> 16) & 0xf;
    printd("and r%u, r%u, ", rd, rn);
    arm.r[rd] = arm.r[rn] & dp_asr_imm(opcode);
}

/* AND Rd, Rn, Rm, ROR # */
static void and_ror_imm(uint32_t opcode)
{
    uint32_t rd = (opcode >> 12) & 0xf;
    uint32_t rn = (opcode >> 16) & 0xf;
    printd("and r%u, r%u, ", rd, rn);
    arm.r[rd] = arm.r[rn] & dp_ror_imm(opcode);
}

/* AND Rd, Rn, Rm, LSL Rs */
static void and_lsl_reg(uint32_t opcode)
{
    uint32_t rd = (opcode >> 12) & 0xf;
    uint32_t rn = (opcode >> 16) & 0xf;
    printd("and r%u, r%u, ", rd, rn);
    arm.r[rd] = arm.r[rn] & dp_lsl_reg(opcode);
}

/* AND Rd, Rn, Rm, LSR Rs */
static void and_lsr_reg(uint32_t opcode)
{
    uint32_t rd = (opcode >> 12) & 0xf;
    uint32_t rn = (opcode >> 16) & 0xf;
    printd("and r%u, r%u, ", rd, rn);
    arm.r[rd] = arm.r[rn] & dp_lsr_reg(opcode);
}

/* AND Rd, Rn, Rm, ASR Rs */
static void and_asr_reg(uint32_t opcode)
{
    uint32_t rd = (opcode >> 12) & 0xf;
    uint32_t rn = (opcode >> 16) & 0xf;
    printd("and r%u, r%u, ", rd, rn);
    arm.r[rd] = arm.r[rn] & dp_asr_reg(opcode);
}

/* AND Rd, Rn, Rm, ROR Rs */
static void and_ror_reg(uint32_t opcode)
{
    uint32_t rd = (opcode >> 12) & 0xf;
    uint32_t rn = (opcode >> 16) & 0xf;
    printd("and r%u, r%u, ", rd, rn);
    arm.r[rd] = arm.r[rn] & dp_ror_reg(opcode);
}

#define INSTR_DP_REG(op)                                                      \
    op##_lsl_imm, op##_lsl_reg, op##_lsr_imm, op##_lsr_reg, op##_asr_imm,     \
        op##_asr_reg, op##_ror_imm, op##_ror_reg, op##_lsl_imm, op##_lsl_reg, \
        op##_lsr_imm, op##_lsr_reg, op##_asr_imm, op##_asr_reg, op##_ror_imm, \
        op##_ror_reg

typedef void (*instr_t)(uint32_t opcode);

static instr_t instr[0xfff] = {
    /* 0x000 */
    INSTR_DP_REG(and)
    /* 0x010 */
};

void arm7_init(void)
{
    memset(&arm, 0, sizeof(arm));
    arm7_reset();
}

void arm7_reset(void)
{
    arm.r14_svc = arm.r[PC];
    arm.spsr_svc = arm.cpsr;
    arm.cpsr = ARM7_PSR_IRQ_DISABLE | ARM7_PSR_FIQ_DISABLE | ARM7_PSR_SVC_MODE;
    arm.r[PC] = 0;
}

static uint32_t arm7_fetch(void)
{
    uint32_t pc = arm.r[PC];
    uint32_t addr = arm.cpsr & ARM7_PSR_STATE_BIT ? pc >> 1 : pc >> 2;
    return mmu_read_word(addr);
}

static void arm7_execute(void)
{
    uint32_t opcode = arm7_fetch();
    if (evaluate_cond(opcode >> 28, arm.cpsr)) {
        uint32_t code = ((opcode >> 16) & 0xff0) | ((opcode >> 4) & 0x0f);
        printd("[0x%.8x][0x%.3x] ", opcode, code);
        instr[code](opcode);
    }
}

void arm7_step(void)
{
    arm7_execute();
}
